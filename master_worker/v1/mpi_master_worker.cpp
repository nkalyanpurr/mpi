#include <mpi.h>
#include <iostream>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif


int main (int argc, char* argv[]) {
 int startinterval = 0;
MPI_Init(&argc, &argv);
  if (argc < 6) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  int fid = atoi(argv[1]);
  float a = atof(argv[2]);
  float b = atof(argv[3]);
  int n = atoi(argv[4]);
  int intensity = atoi(argv[5]);
  int rank, size;
  float globalSum;
  float result;
  MPI_Status status;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  int count = 0;
  int chunk= n/size;
  int* arr = new int[2];
  int TAG;
  if(rank == 0)
  {          
     	for(int i = 1; i < size; i++)
	{
                arr[0] = startinterval;
		arr[1] = startinterval + chunk;
		TAG = 1;
		count++; 
		if(startinterval > n)
		{
			TAG = 0;
			count--;
		}         
        	else if( (startinterval+chunk) > n)
			arr[1] = n;
        	MPI_Send( arr, 2, MPI_INT, i, TAG, MPI_COMM_WORLD);
		startinterval += chunk;	
	}

      	while(count != 0)
     	{
  		MPI_Recv(&result, 1, MPI_FLOAT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
		if(status.MPI_TAG == 0)
        	{
        		 count--;
		}

		count--;	
 		globalSum += result;
       		arr[0] = startinterval;
		arr[1] = startinterval + chunk;
		TAG = 1;
		count++;
 		if(startinterval >= n)
		{
	 		TAG = 0;
			count--;
		}               

        	else if( (startinterval+chunk) > n)
			arr[1] = n;
        	MPI_Send( arr, 2, MPI_INT, status.MPI_SOURCE, TAG, MPI_COMM_WORLD);
		startinterval += chunk;
	}

  }


  else
  { 

	int* B = new int[2];
  	MPI_Recv(B, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		
  	while(&status.MPI_TAG!=0)
  	{
		float res = 0.0;
		if(B[1] > n)
		{
			MPI_Send(&res, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
			break;
		}
		else if(B[1] <= n)
		{

	   	   for(int i= B[0]; i < B[1]; i++)
	   	   {
	 		float x = a + (i + 0.5)*((b-a)/n);

     			if( fid == 1)
  			{
 				res += f1(x,intensity);
  			}
  			else if( fid == 2)
  			{
      				res += f2(x,intensity);
  			}
   			else if( fid == 3)
    			{
      				res += f3(x,intensity);
  			}
  			else if( fid == 4)
    			{
      				res += f4(x,intensity);
  			}
		
            	   }

 		MPI_Send(&res, 1, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);

	 	MPI_Recv(B, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	}
    }
}
 

if(rank == 0){
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  float elapsedTime = (end.tv_nsec - start.tv_nsec) / 1000000000.0 + (end.tv_sec - start.tv_sec);
  globalSum = globalSum * ((b-a)/n);  
  cout<< globalSum;
  cerr <<  elapsedTime << endl;
}

  MPI_Finalize();
  return 0;
}
