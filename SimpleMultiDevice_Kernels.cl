__kernel void multiDeviceKernel(uint start, __global uint *input,
                                __global float *output)
{

	//if(get_global_id(0)%10000 == 0) printf("start: %d\n", start);

    uint tid = get_global_id(0);  // which thread 0 to 65535 inside the gpu this is
	
	//if(get_global_id(0)%10000 == 0) printf("tid: %d\n", tid);
	
	bool print = 0;
	

	
	uint ruleCode = tid + start;
	
	//if(ruleCode > 65536 * 4) printf(" rulecode: %d\n", ruleCode);
	
	//	print = 1;
	
	//	for(int i = 0; i < 100; i++) printf("%d",input[i]);
	//	printf("\n");
			
	//}
	
	
	uint checkTemp = ruleCode;
	
	uint rule[32];
	
	for(int i = 0; i < 32; i++){
	
		rule[i] = ruleCode % 2;
		ruleCode = ruleCode / 2;
	
		//if(print) printf("%d \b", rule[i]);
	
	}
	
	if(print) printf("%d\b\n", rule[0]);
	
	// automation arrays
	
	uint normalAuto0[100];
	uint normalAuto1[100];
	
	uint diffAuto0[100];
	uint diffAuto1[100];


	
	// neighborhood values for calculating next cell
	
	float diff[100]; // record differences

	int nl2;
	int nl;
	int nm;
	int nr;
	int nr2;
	
	int nHood;
	
	int dl2;
	int dl;
	int dm;
	int dr;
	int dr2;
	
	uint dHood;
	
	float lineDiff;
	float autoDiff;
	float extinction = 0;
	float lastline;
	
	for(int m = 0; m < 100; m++){
	
		autoDiff = 0;
	
		for(int s = 0; s < 100; s++){
	
		
			normalAuto0[s] =input[s];
			diffAuto0[s] = input[s];
		
			if(s == m){			// provide difference between automations
		
				if(diffAuto0[m] == 1) diffAuto0[m] = 0;
			    else                  diffAuto0[m] = 1;
			}
	
		}
	
		lastline = 1;
	
		for (int i = 0; i < 50; i++){
	
			lineDiff = 0.0;
	
			for(int j = 0; j < 100; j++){
		
				// get local neigborhoods
		
				nl2 = j - 2;
				dl2 = j - 2;
		
				if(nl2 < 0){
			
					nl2 += 100;
					dl2 += 100;
				}
			
				nl = j - 1;
				dl = j - 1;
		
				if(nl < 0){
			
					nl += 100;
					dl += 100;
				}
			
				nm = j;
				dm = j;
		
				nr = j + 1;
				dr = j + 1;
			
				if(nr > 99){
			
					nr -= 100;
					dr -= 100;
				}
			
				nr2 = j + 2;
				dr2 = j + 2;
			
				if(nr2 > 99){
			
					nr2 -= 100;
					dr2 -= 100;
				}
			
				if(i % 2 == 0){
			
					nl2 =  normalAuto0[nl2];
					dl2 =    diffAuto0[dl2];
					nl  =  normalAuto0[nl];
					dl  =    diffAuto0[dl];
					nm  =  normalAuto0[nm];
					dm  =    diffAuto0[dm];
					nr  =  normalAuto0[nr];
					dr  =    diffAuto0[dr];
					nr2 =  normalAuto0[nr2];
					dr2 =    diffAuto0[dr2];
			
				}else{
			
					nl2 =  normalAuto1[nl2];
					dl2 =    diffAuto1[dl2];
					nl  =  normalAuto1[nl];
					dl  =    diffAuto1[dl];
					nm  =  normalAuto1[nm];
					dm  =    diffAuto1[dm];
					nr  =  normalAuto1[nr];
					dr  =    diffAuto1[dr];
					nr2 =  normalAuto1[nr2];
					dr2 =    diffAuto1[dr2];
			
				}
			
				//if (print) printf("%d \b", nm);
			
					nHood = nl2 * 16 + nl * 8 + nm * 4 + nr * 2 + nr2;
					dHood = dl2 * 16 + dl * 8 + dm * 4 + dr * 2 + dr2;
		
				if(nHood != dHood){
					lineDiff = lineDiff + 1.0;
					//autoDiff = autoDiff + 1;
					//if(print) printf("O");
				}
				//else if(rule[nHood] == 0 & print) printf(" ");
				//else if(print) printf("X");
			
				if( i % 2 == 0){
			
					normalAuto1[j] = rule[nHood];
					  diffAuto1[j] = rule[dHood];
			
				} else {
				
					normalAuto0[j] = rule[nHood];
					  diffAuto0[j] = rule[dHood];
				  
				}

				if(print) printf("\n");

			}  // single interation finish
		
			if(! lineDiff){
				
				//extinction = extinction + lastline + .1 * (i - 1); // - ((50 - i)/3);
				
				
				break;
			}
			autoDiff += lineDiff;
			lastline = lineDiff;
		
		} // single automation finish
	
		//if(print) printf(" extinction: %f\n", extinction);
		diff[m] = autoDiff;
	
	}  // 100 automation finish
	
	// take derivative of change
	
	float mean = diff[0];
	
	float tempDiff = diff[99];
	
	diff[99] = diff[99] - diff[98];
	
	for(int i = 98; i > 0; i--){
		
		tempDiff  = ( tempDiff - diff[i-1] ) / 2.0;
		float swapTemp = diff[i];
		diff[i] = tempDiff;
		tempDiff = swapTemp;
		mean = mean + diff[i];
		
	}
	
	diff[0] = tempDiff - diff[0];
	
	mean = mean / 100;
	
	float var = 0;
	
	for(int i = 0; i < 100; i++){
	
		var = var + pow((diff[i] - mean), 2);
	
	}
	
	var = var / 99;
	
    output[tid] =  var; //* extinction;   // variance squared minus the final difference seems to be a good way to keep track of the trend vs the complexity
	
	
}
