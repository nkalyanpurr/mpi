#include <mpi.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char*argv[]) {
	int rank, size;
	MPI_Init(&argc, &argv);
	char hostname[128] = "";
	gethostname(hostname,size_t(hostname));
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	printf("I am process %d out of %d. I am running on %s \n", rank, size, hostname);
	MPI_Finalize();
}
