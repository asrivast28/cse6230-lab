#include <assert.h>
#include <mpi.h>

/**
 *  Broadcast the data using the minimum spanning tree algorithm,
 *  i.e., rank 0 has the initial data and we recursively double the
 *  number of communicating processes over log_2(P) rounds.
 *
 *  \pre Number of processes, P, must divide the message length, len.
 */
void bcast (int* data, const int len)
{
  int P, rank;
  MPI_Comm_size (MPI_COMM_WORLD, &P);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  assert ((len % P) == 0);
  const int len_per_proc = len / P;

  /* Iterate over log_2(P) rounds */
  for (int offset = P/2; offset >= 1; offset >>= 1) {
    if ((rank % offset) == 0) {
      if (((rank / offset) % 2) == 0) {
	const int rank_dest = rank + offset;
	const int tag = 1000 + rank + offset;
	MPI_Send (data, len, MPI_INT, rank_dest, tag, MPI_COMM_WORLD);
      } else {
	const int rank_source = rank - offset;
	const int tag = 1000 + rank;
	MPI_Status stat;
	MPI_Recv (data, len, MPI_INT, rank_source, tag, MPI_COMM_WORLD, &stat);
      }
    }
  }
}

const char* bcast_algorithm (void)
{
  static const char* name = "tree";
  return name;
}

/* eof */
