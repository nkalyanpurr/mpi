#include <mpi.h>
#include <iostream>
#include <chrono>

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
    MPI_Init(&argc, &argv);

    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    if (argc < 6) {
        std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
        return -1;
    }
    
    int fid = atoi(argv[1]);
    float a = atof(argv[2]);
    float b = atof(argv[3]);
    int n = atoi(argv[4]);
    int intensity = atoi(argv[5]);
    long chunk = ((n/size)/8);
    float sum = 0.0;
    float globalSum = 0.0;
    int startinterval = 0;
    int src;
    int count = 0;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    if(rank == 0)
    {
        for(int i = 1; i < size; i++)
        {
            if(startinterval != n)
            {
                MPI_Send(&startinterval, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
                startinterval = startinterval + (chunk*3);
                if(startinterval > n)
                    startinterval  = n;
            }
        }
    }

    while(startinterval <= n)
    {
        if(rank == 0)
        {
            float result = 0.0;
            MPI_Recv(&result, 1, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            if(result != 1)
                sum += result;
            else
            {
                count ++;
                if(count == (size - 1))
 		{
 			break;
		}
            }

            if(startinterval < n)
   	    {
               MPI_Send(&startinterval, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
               startinterval += (chunk*3);
	    }
            else if(startinterval >= n)
            {
                startinterval = n;
                MPI_Send(&startinterval, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            }
            if(startinterval > n)
                startinterval = n;
        }
        else
        {
            MPI_Recv(&startinterval, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            if(startinterval != n)
            { 
   	      for(int it = 0; it < 3; it++)
		{
                int lowerbound = startinterval;
                int upperbound = lowerbound + chunk;
                if(upperbound > n)
                    upperbound = n;
                float res = 0.0;
                 for(int i= lowerbound; i < upperbound; i++)
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
     
                MPI_Send(&res, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
		startinterval = upperbound;
		}
              
            }
            else if(startinterval == n)
             {
              float result = 1;
              MPI_Send(&result, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
              startinterval += 5;
    	      break;
           }
       }
    }

   if(rank == 0)
    {
	globalSum = sum*((b-a)/n);
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);
	cout<<globalSum;
	float elapsedTime = (end.tv_nsec - start.tv_nsec) / 1000000000.0 + (end.tv_sec - start.tv_sec);  cerr <<  elapsedTime << endl;
    }

    MPI_Finalize();
    return 0;
}
