#include <mpi.h>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <chrono>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

  int check2DHeat(double** H, long n, long rank, long P, long k);
//this assumes array of array and grid block decomposition

#ifdef __cplusplus
}
#endif

/***********************************************
 *         NOTES on check2DHeat.
 ***********************************************
 *
 *  First of, I apologize its wonky.
 *
 *  Email me ktibbett@uncc.edu with any issues/concerns with this.
Dr. Saule or the other
 *    TA's are not familiar with how it works.
 *
 * Params:
 *  n - is the same N from the command line, NOT the process's part of N
 *  P - the total amount of processes ie what MPI_Comm_size gives you.
 *  k - assumes n/2 > k-1 , otherwise may return false negatives.
 *
 *
 * Disclaimer:
 ***
 *** Broken for P is 9. Gives false negatives, for me it was always
 ***  ranks 0, 3, 6. I have not found issues with 1, 4, or 16, and these
 ***  are what `make test` will use.
 ***
 *
 * Usage:
 *  When code is WRONG returns TRUE. Short example below
 *  if (check2DHeat(...)) {
 *    // oh no it is (maybe) wrong
 *    std::cout<<"rank: "<<rank<<" is incorrect"<<std::endl;
 *  }
 *
 *
 *
 *  I suggest commenting this out when running the bench
 *
 *
 * - Kyle
 *
 *************/


// Use similarily as the genA, genx from matmult assignment.
double genH0(long row, long col, long n) {
  double val = (double)(col == (n/2));
  return val;
}


int main(int argc, char* argv[]) {

  if (argc < 3) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <N> <K>"<<std::endl;
    return -1;
  }
  long N = atol(argv[1]);
  long K = atol(argv[2]);
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int rootp = sqrt(size);
  int blockSize = N/rootp;
  int rowBegin = (rank - (rank - (rank / rootp))) * blockSize;
  int rowEnd = rowBegin + (blockSize-1);
  int mod  = fmod(rank, rootp);
  int columnBegin = (rank - (rank - mod)) * blockSize;
  int columnEnd = columnBegin + (blockSize-1);

  int count = 0;
  double *H = new double[blockSize*blockSize];
  double *Hprev = new double[blockSize*blockSize];
  int check = 0;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  for(int r = rowBegin; r < rowEnd; r++) {
    for(int c = columnBegin; c < columnEnd; c++) {
      Hprev[check]= genH0(r, c, N);
      check++;
    }

  }


  for(long it = 0; it < K; it++) {
	double *rbuff1 = new double[rootp];
	double *rbuff2 = new double[rootp];
	double *rbuff3 = new double[rootp];
	double *rbuff4 = new double[rootp];
	double *lbuff1 = new double[rootp];
	double *lbuff2 = new double[rootp];
	double *lbuff3 = new double[rootp];
	double *lbuff4 = new double[rootp];
	int rowsize = 0;
	int rowcount = 0;
	MPI_Request request[rowsize];
	MPI_Status status[4];

    	if(columnBegin != 0){
    		rowsize += 2;
    	}
    	if(rowBegin != 0){
    		rowsize += 2;
    	}
    	if(columnEnd != N-1){
    		rowsize += 2;
    	}
    	if(rowEnd != N-1){
    		rowsize += 2;
    	}
    	if(columnBegin != 0){
	      int c = 0;
        for(int i = 0; i < rootp; i++) {
        	lbuff1[i] = Hprev[c];
        	c += blockSize;     
        }

      	MPI_Isend(lbuff1, rootp, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &request[rowcount]);
      	rowcount++;
      	MPI_Irecv(rbuff1, rootp, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &request[rowcount]);
      	rowcount++;
      	count+=2;
    }

    if(rowBegin != 0){
     	 int r = 0;
      for(int i = 0; i < rootp; i++) {
		lbuff2[i] = Hprev[r];
        	r++;     
      }

      MPI_Isend(lbuff2, rootp, MPI_DOUBLE, rank - rootp, 0, MPI_COMM_WORLD, &request[rowcount]);
      rowcount++;
      MPI_Irecv(rbuff2, rootp, MPI_DOUBLE, rank - rootp, 0, MPI_COMM_WORLD, &request[rowcount]);
      count+=2;
      rowcount++;
    }

    if(columnEnd != N-1){
      int c = columnEnd;
      for(int i =0; i<rootp; i++) {
        	lbuff3[i]= Hprev[r];
        	c += blockSize;     
      }

      MPI_Isend(lbuff3, rootp, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &request[rowcount]);
      rowcount++;
      MPI_Irecv(rbuff3, rootp, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &request[rowcount]);
      rowcount++;
      count+=2;
    }

    if(rowEnd != N-1){
      int r = rowEnd;
      for(int i = 0; i < rootp; i++) {
        	lbuff4[i] = Hprev[c];
        	r++;     
      }

      MPI_Isend(lbuff4, rootp, MPI_DOUBLE, rank + rootp, 0, MPI_COMM_WORLD, &request[rowcount]);
      rowcount++;
      MPI_Irecv(rbuff4, rootp, MPI_DOUBLE, rank + rootp, 0, MPI_COMM_WORLD, &request[rowcount]);
      count += 2;
      rowcount++;
    }
   
    MPI_Waitall(rowsize, request, MPI_STATUSES_IGNORE);
    int pointer = 0;

  for(int i = 0; i< blockSize; i++) {
    for(int  j = 0; j<blockSize; j++) {

        if(i == 0 && j==0) {
          if(rowBegin == 0 && columnBegin ==0){
            	H[pointer] = (Hprev[1] + Hprev[(i+1)*blockSize+j] +  (3 * Hprev[i*blockSize+j])) / 5;
          }
	  else{
            	H[pointer] = (Hprev[1] + Hprev[(i+1)*blockSize+j] +  Hprev[i*blockSize+j] + rbuff1[0] + rbuff2[0]) / 5;
          }
         
        }
       else if(i==blockSize-1 && j==blockSize-1) {

         if(rowEnd == N-1 && columnEnd ==N-1){
        	H[pointer]= (Hprev[i*blockSize+(j-1)] + Hprev[(i-1)*blockSize+j] + (3 * Hprev[i*blockSize+j]))/5;

         }
	 else{
        	H[pointer]= (Hprev[i*blockSize+(j-1)] + Hprev[(i-1)*blockSize+j] + Hprev[i*blockSize+j] + rbuff3[i] + rbuff4[i])/5;

        }
      } 
    else if(i==0 && j!=blockSize-1) {
      	if(rowBegin == 0 && columnBegin !=N-1){
        	H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[i*blockSize+(j-1)] + Hprev[(i+1)*blockSize+j]
         + (2 * Hprev[i*blockSize+j]))/5;

        }
	else{
        	H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[i*blockSize+(j-1)] + Hprev[(i+1)*blockSize+j]
         + Hprev[i*blockSize+j] + rbuff2[j])/5;

        }
     
    } 
    else if(i!=0 && j==0) {
     	if(rowBegin != 0 && columnBegin ==0){
      		H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[(i-1)*blockSize+j] + Hprev[(i+1)*blockSize+j]
       + (2 * Hprev[i*blockSize+j]))/5;               
      }
      else{
       		H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[(i-1)*blockSize+j] + Hprev[(i+1)*blockSize+j]
       + Hprev[i*blockSize+j] + rbuff1[i])/5;  
    }
  } 
    else if(i!=N-1 && j==N-1) {
    	if(rowEnd != N-1 && columnEnd ==N-1){
    	  	H[pointer]= (Hprev[i*blockSize+(j-1)] + Hprev[(i-1)*blockSize+j] + Hprev[(i+1)*blockSize+j]
       + (2 * Hprev[i*blockSize+j]))/5;              
        }
	else{
      		H[pointer]= (Hprev[i*blockSize+(j-1)] + Hprev[(i-1)*blockSize+j] + Hprev[(i+1)*blockSize+j]
       + Hprev[i*blockSize+j] + rbuff3[i])/5; 
    }
  } 
   else if(i==N-1 && j!=N-1) {
    	if(rowEnd == N-1 && columnEnd !=N-1){
      		H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[i*blockSize+(j-1)] + Hprev[(i-1)*blockSize+j]
       + (2 * Hprev[i*blockSize+j]))/5;          
        }
        else{
      		H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[i*blockSize+(j-1)] + Hprev[(i-1)*blockSize+j]
       + Hprev[i*blockSize+j] + rbuff4[j])/5;
    }
  } 
   else if(i==0 && j==N-1) {
    	if(rowBegin == 0 && columnEnd ==N-1){
      		H[pointer]= (Hprev[i*blockSize+(j-1)] + Hprev[(i+1)*blockSize+j] + (3 * Hprev[i*blockSize+j]))/5;
    	}
	else{
      		H[pointer]= (Hprev[i*blockSize+(j-1)] + Hprev[(i+1)*blockSize+j] + Hprev[i*blockSize+j] + rbuff2[j] + rbuff3[i])/5;
    }
  } 
  else if(i==N-1 && j==0) {
    	if(rowEnd == N-1 && columnBegin ==0){
      		H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[(i-1)*blockSize+j] + (3 * Hprev[i*blockSize+j]))/5;
    	}
	else{
      		H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[(i-1)*blockSize+j] + Hprev[i*blockSize+j] + rbuff1[i] + rbuff4[j])/5;
    }
  }
	else {
    		H[pointer]= (Hprev[i*blockSize+(j+1)] + Hprev[(i-1)*blockSize+j] + Hprev[i*blockSize+j] +
     		Hprev[(i+1)*blockSize+j] + Hprev[i*blockSize+(j-1)])/5;
  }
  pointer++;
}
}
Hprev = H;
}
if(rank == 0){   
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  float elapsedTime = (end.tv_nsec - start.tv_nsec) / 1000000000.0 + (end.tv_sec - start.tv_sec);
  cerr <<  elapsedTime << endl;
}


MPI_Finalize();

return 0;
}

