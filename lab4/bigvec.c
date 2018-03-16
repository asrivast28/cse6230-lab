#include <assert.h>
#include <mpi.h>
#include <stdlib.h>

static void scatter (int* data, const int len);
static void allgather (int* data, const int len);

/**
 *  Broadcast the data using the minimum spanning tree algorithm,
 *  i.e., rank 0 has the initial data and we recursively double the
 *  number of communicating processes over log_2(P) rounds.
 *
 *  \pre Number of processes, P, must divide the message length, len.
 */
void bcast (int* data, const int len)
{
  scatter (data, len);
  allgather (data, len);
}

static void scatter (int* data, const int len)
{
  int P, rank;
  MPI_Comm_size (MPI_COMM_WORLD, &P);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  assert ((len % P) == 0);
  const int len_per_proc = len / P;

  for (int offset = P/2; offset >= 1; offset >>= 1) {
    if ((rank % offset) == 0) {
      if (((rank / offset) % 2) == 0) {
        int* msg_send = data + (rank + offset)*len_per_proc;
        const int msg_len = len_per_proc * offset;
        const int rank_dest = rank + offset;
        const int msg_tag = 1000 + rank_dest; /* must match receiver's! */
        MPI_Send (msg_send, msg_len, MPI_INT, rank_dest, msg_tag, MPI_COMM_WORLD);
      } 
      else {
        MPI_Status stat;
        int* msg_recv = data + rank*len_per_proc;
        const int msg_len = len_per_proc * offset;
        const int rank_source = rank - offset;
        const int msg_tag = 1000 + rank; /* must match sender's! */
        MPI_Recv (msg_recv, msg_len, MPI_INT, rank_source, msg_tag, MPI_COMM_WORLD, &stat);
      }
    }
  }
}

static void allgather (int* data, const int len)
{
  int P, rank;
  MPI_Comm_size (MPI_COMM_WORLD, &P);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  assert ((len % P) == 0);
  const int len_per_proc = len / P;

  const int rank_next = (rank + 1) % P;
  const int rank_prev = (rank + P - 1) % P;

  MPI_Request* requests = (MPI_Request*) malloc (P * sizeof (MPI_Request));

  for (int offset = P; offset > 0; --offset) {
    int* msg_send = data + ((rank + offset) % P) * len_per_proc; 
    int* msg_recv = data + ((rank + offset + P - 1) % P) * len_per_proc;
    const int send_tag = 1000 + rank + offset;
    const int recv_tag = 1000 + rank_prev + offset;
    MPI_Status stat;
    MPI_Isend (msg_send, len_per_proc, MPI_INT, rank_next, send_tag, MPI_COMM_WORLD, &requests[offset-1]);
    MPI_Recv (msg_recv, len_per_proc, MPI_INT, rank_prev, recv_tag, MPI_COMM_WORLD, &stat);
  }
  MPI_Status *statuses = (MPI_Status*) malloc (P * sizeof(MPI_Status));
  MPI_Waitall (P, requests, statuses);
  free (requests);
  free (statuses);
}

const char* bcast_algorithm (void)
{
  static const char* name = "bigvec";
  return name;
}

/* eof */
