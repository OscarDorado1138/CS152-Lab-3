
// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 32 kilobytes available
// for the CBP-2 contest; it is just an example.

class my_update : public branch_update {
public:
	unsigned int index;
};

class my_predictor : public branch_predictor {
public:
#define WEIGHT_LENGTH 7
#define WEIGHT_TABLE_LENGTH 512
#define HISTORY_LENGTH	20

	// Default Global Variables
	my_update u;
	branch_info bi;
	unsigned int history;


	// New Global Variables

	/* Index of Weight Matrix */
	unsigned int i;           
	/* Perceptron Output */
	int y_out;
	/*shift register that records the outcomes of branches 
	  as they are executed, or speculatively as they 
	  are predicted.
	*/
	int shift_register[HISTORY_LENGTH+1];  // shift register recording the non-speculate partial sums, additional one bit is for the bias
	/* integers representing the addresses of the last h 
 	   branches predicted modulo n */
	int v[HISTORY_LENGTH];   
	/* the weight matrix */
	int weights[WEIGHT_TABLE_LENGTH][HISTORY_LENGTH+1]; // the weight matrix
	/* Threshold */
	// Best accuracy: Theta = 1.93h + 14
	// 42.95
	const int theta;





	my_predictor (void) : history(0), theta(int(1.93*(HISTORY_LENGTH) + 14))
	{ 
		memset(weights, 0, sizeof(weights));
		memset(shift_register, 0, sizeof(shift_register));
		memset(v, 0, sizeof(v));
	}



	branch_update *predict (branch_info & b) 
	{
		bi = b;
		if(b.br_flags & BR_CONDITIONAL){
		    u.index = history ^ (b.address & ((1<<HISTORY_LENGTH) -1));
		 

		    //Prediction Function
		    // i = pc mod n
		    i = b.address % WEIGHT_TABLE_LENGTH;
		    // y_out = SR[h] + W[i,0]
		    y_out = shift_register[HISTORY_LENGTH] + weights[i][0];

		    if(y_out >= 0)
		        u.direction_prediction(true);
		    else
		        u.direction_prediction(false);

		}
		else{
		    u.direction_prediction(true);
		}
		u.target_prediction (0);
		return &u;

	}



	void update (branch_update *u, bool taken, unsigned int target) 
	{
		if (bi.br_flags & BR_CONDITIONAL) 
		{

			// Update the next h partial sums.
		    for(unsigned int j = 1; j <= HISTORY_LENGTH; ++j)
		    {
		        unsigned int k_j = HISTORY_LENGTH - j;
				// Prediction = taken
				if(taken)
			    	shift_register[k_j+1] = shift_register[k_j] + weights[i][j];
				else
			    	shift_register[k_j+1] = shift_register[k_j] - weights[i][j];
		    }
		    shift_register[0] = 0;



		    // Procedure train
		    // Update the weight matrix:
		    // If incorrect or y_out below threshold then adjust weights
		    if(u->direction_prediction() != taken || abs(y_out) <= theta)
		    {
				if(taken == true)
				{
					if(weights[i][0] < (1 << (WEIGHT_LENGTH-1) ))
						weights[i][0] += 1;
				}
				else{
					if(weights[i][0] > (1 - 1*(1 << (WEIGHT_LENGTH-1) )))
						weights[i][0] -= 1;
				}

				for(unsigned int j = 1; j <= HISTORY_LENGTH; ++j)
				{
			    	unsigned int k_j = v[j-1];
			    	int outcome = 1 & (history >> (j-1) );
					if(taken == outcome)
					{
						if(weights[k_j][j] < (1 << (WEIGHT_LENGTH-1) ))
							weights[k_j][j] += 1;
					}
					else{
						if(weights[k_j][j] > (1 - 1*(1 << (WEIGHT_LENGTH-1) )))
							weights[k_j][j] -= 1;
					}
	
					

				}
		    }
		    

		    for(unsigned int j = 1; j < HISTORY_LENGTH; ++j)
		    {
		        unsigned int k_j = HISTORY_LENGTH - j;
		        v[k_j] = v[k_j-1];
		    }
		    v[0] = i;


		    // Default Code
		    history <<= 1;
		    history |= taken;
		    history &= (1<<HISTORY_LENGTH)-1;
		}
	}

};
