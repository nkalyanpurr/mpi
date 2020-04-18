#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
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
  
  if (argc < 6) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
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
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  float result=0.0;
  for(int i=rank; i < n; i+=size)
  {

	 	float x = a + (i + 0.5)*((b-a)/n);

     		if( fid == 1)
  		{
 			result += f1(x,intensity);
  		}
  		else if( fid == 2)
  		{
      			result += f2(x,intensity);
  		}
   		else if( fid == 3)
    		{
      			result += f3(x,intensity);
  		}
  		else if( fid == 4)
    		{
      			result += f4(x,intensity);
  		}

  }
  MPI_Reduce(&result,&globalSum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  float elapsedTime = (end.tv_nsec - start.tv_nsec) / 1000000000.0 + (end.tv_sec - start.tv_sec);
  
  if(rank == 0)
{
        globalSum = globalSum * ((b-a)/n);
  	cout<<globalSum;
  	cerr << elapsedTime << endl;
}
  MPI_Finalize();

  return 0;
}
