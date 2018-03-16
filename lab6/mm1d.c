/**
 *  \file mm1d.c
 *  \brief Implements a 1-D distributed block row matrix multiply
 *  operation, C += A * B.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>
#include "mpi_fprintf.h"
#include "mpi_assert.h"

#define MINTRIALS 3 /* Minimum number of timing trials */
#define MINTIME 1.0 /* Minimum time (in seconds) */

/* ==================================================
 * "Sequential" (single-node, non-MPI) matrix operations.
 */

/** Creates a newly-allocated matrix of size m x n. */
extern float* mm_create (int m, int n);

/** Frees a matrix. */
extern void mm_free (float* A);

/**
 *  Computes C += A * B, where A is an m x k matrix, B is a k x n
 *  matrix, and C is an m x n matrix. Each of A, B, and C is stored in
 *  column-major order with leading dimemsion lda, ldb, and ldc,
 *  respectively.
 */
extern void mm_local (const int m, const int n, const int k,
		      const float* A, const int lda,
		      const float* B, const int ldb,
		      float* C, const int ldc);

/* ==================================================
 */

/**
 *  Implements a 1-D distributed block row matrix multiply operation,
 *  C += A * B.
 *
 *  The global matrix operands are each 'n x n'. Each of the 'P' MPI
 *  processes in the communicator 'comm' owns 'n / P' consecutive rows
 *  of each operand. The locally owned portions of the matrix are
 *  'A_local', 'B_local', and 'C_local', stored in column-major
 *  order. The algorithm circularly shifts 'B_local'.
 *
 *  See also slide 28 of the October 23, 2012 CSE 6230 lecture:
 *  http://vuduc.org/teaching/cse6230-hpcta-fa12/slides/cse6230-fa12--distmem.pdf
 *
 *  Returns the total time spent doing just the local matrix
 *  multiplies.
 */
double
mm1d (int n,
      const float* A_local, float* B_local, float* C_local,
      MPI_Comm comm)
{
  double t_comp = 0;

  int P; /* No. of MPI ranks */
  int r; /* Rank of the currently executing process */

  MPI_Comm_size (comm, &P);
  MPI_Comm_rank (comm, &r);

  MPI_Assert (comm, (n % P) == 0);
  int n_by_P = n / P;

#define SEND 0
#define RECV 1
  float* B_buffered[2];
  B_buffered[SEND] = B_local;
  B_buffered[RECV] = mm_create (n_by_P, n);
  MPI_Assert (comm, B_buffered[RECV] != NULL);

  const int r_next = (r + 1) % P;
  const int r_prev = (r + P - 1) % P;

  for (int k = 0; k < P; ++k) {
    /* Do the local multiply */
    double t_start = MPI_Wtime ();
    int offset_A = k * n_by_P * n_by_P;
    mm_local (n_by_P, n, n_by_P,
	      A_local + offset_A, n_by_P,
	      B_local, n_by_P,
	      C_local, n_by_P);
    t_comp += MPI_Wtime () - t_start;

    /* Initiate communication */
    MPI_Status stat;
    const int TAG = 1000;
    MPI_Sendrecv (B_buffered[SEND], n_by_P * n, MPI_FLOAT, r_next, TAG,
		  B_buffered[RECV], n_by_P * n, MPI_FLOAT, r_prev, TAG,
		  comm, &stat);

    /* Swap buffers for next iteration */
    float* tmp = B_buffered[SEND];
    B_buffered[SEND] = B_buffered[RECV];
    B_buffered[RECV] = tmp;

  	if (B_buffered[SEND] != B_local)
    	memcpy (B_local, B_buffered[SEND], n_by_P * n * sizeof (float));
  }

  return t_comp;
}

/** Initializes matrix with uniformly random values in [0, 1]. */
void
init_mat_random (int m, int n, float* A)
{
  assert (A || !(m*n));
  for (int i = 0; i < m; ++i) { /* loop over rows */
    for (int j = 0; j < n; ++j) { /* loop over columns */
      A[i + j*m] = drand48 ();
    }
  }
}

int
main (int argc, char* argv[])
{
  MPI_Init (&argc, &argv);

  const MPI_Comm comm = MPI_COMM_WORLD;
  int P /* No. of procs */, r /* local rank */;
  MPI_Comm_size (comm, &P);
  MPI_Comm_rank (comm, &r);

  const int n_desired = 4096; /* Target problem size */
  const int n_local = (n_desired + P - 1) / P; /* ceil (n_desired / P) */
  const int n = n_local * P; /* Actual global problem size */

  MPI_fprintf (comm, stderr, "Creating %d x %d local problem...\n", n_local, n);
  float* A_local = mm_create (n_local, n);
  float* B_local = mm_create (n_local, n);
  float* C_local = mm_create (n_local, n);
  init_mat_random (n_local, n, A_local);
  init_mat_random (n_local, n, B_local);
  init_mat_random (n_local, n, C_local);

  MPI_fprintf (comm, stderr, "Performing one warm-up multiply...\n");
  MPI_Barrier (comm);
  mm1d (n, A_local, B_local, C_local, comm);
  MPI_Barrier (comm);

  MPI_fprintf (comm, stderr, "Timing trials...\n");
  int trials = 0;
  double t_elapsed = 0, t_comp = 0, t_start = MPI_Wtime ();
  do {
    t_comp += mm1d (n, A_local, B_local, C_local, comm);
    ++trials;
    t_elapsed = MPI_Wtime () - t_start;
  } while (trials < MINTRIALS || t_elapsed < MINTIME);

  double t_max = -1, t_comp_max = -1;
  MPI_Reduce (&t_elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
  t_max /= trials;
  MPI_Reduce (&t_comp, &t_comp_max, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
  t_comp_max /= trials;
  if (r == 0) {
    printf ("========================================\n");
    printf ("Problem dimension: n = %d\n", n);
    printf ("Number of MPI ranks: %d\n", P);
    printf ("Number of trials: %d\n", trials);
    printf ("Time per trial (max over all processes): %g seconds\n", t_max);
    printf ("Computation time per trial (max over all processes): %g seconds\n", t_comp_max);
    printf ("Effective performance: %.1f GFLOP/s\n", 2e-9 * n * n * n / t_max);
    printf ("========================================\n");
  }

  MPI_Finalize ();
  return 0;
}

/* eof */

