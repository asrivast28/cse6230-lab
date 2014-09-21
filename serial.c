#include <mpi.h>

/**
 *  Broadcast the data using a serial algorithm (rank 0 sends a
 *  message, one at a time, to all other processes).
 */
void bcast (int* data, const int len)
{
  static const int MSGTAG = 1000;
  int P, rank;
  MPI_Comm_size (MPI_COMM_WORLD, &P);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    for (int dest = 1; dest < P; ++dest)
      MPI_Send (data, len, MPI_INT, dest, MSGTAG, MPI_COMM_WORLD);
  } else { /* rank > 0 */
    MPI_Status stat;
    MPI_Recv (data, len, MPI_INT, 0, MSGTAG, MPI_COMM_WORLD, &stat);
  }
}

const char* bcast_algorithm (void)
{
  static const char* name = "serial";
  return name;
}

/* eof */
