/**
 *  \file bcast-driver.c
 *
 *  \brief Driver file for CSE 6230, Fall 2012, Lab 9:
 *         Broadcast benchmark
 *
 *  \author Rich Vuduc <richie@gatech...>
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <mpi.h>
#include "mpi_fprintf.h"

extern const char* bcast_algorithm (void);
extern void bcast (int* data, const int len);

/** Smallest message to test (in words) */
#define MIN_MSGLEN 1

/** Largest message to test (in words) */
#define MAX_MSGLEN (1 << 24)

/** Minimum time (seconds) for a set of trials */
#define MIN_TIME 0.2

/* ============================================================ */

/**
 *  Initialize the message as it will be used in the pingpong
 *  microbenchmark. Upon return, msgbuf[i] == i on rank 0 and
 *  msgbuf[i] == 0 on all other ranks.
 */
static
void
init_message (const int rank, int* msgbuf, const int len)
{
  if (rank == 0)
    for (int i = 0; i < len; ++i)
      msgbuf[i] = i;
  else
    bzero (msgbuf, len * sizeof (int));
}

/**
 *  Checks (asserts) that every element of the given buffer has its
 *  "expected" value. The expected value is defined as the *initial*
 *  value on the root, i.e., msgbuf[i] == i. See also init_message().
 */
static
int
test_message (const int rank, const int* msgbuf, const int len)
{
  int i;
  for (i = 0; i < len; ++i)
    if (msgbuf[i] != i)
      break;
  return i;
}

/* ============================================================ */

/** Program start */
int
main (int argc, char *argv[])
{
  /* MPI stuff */
  int rank = 0;
  int P = 0;

  /* Output file */
  char outfile[255];
  FILE *fp = NULL; /* output file, only valid on rank 0 */

  /* Message buffer */
  int* msgbuf = NULL;
  int msglen = 0;
  int min_msglen = MIN_MSGLEN;

  /* Start MPI */
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* Get process id */
  MPI_Comm_size (MPI_COMM_WORLD, &P);	/* Get number of processes */
  MPI_fprintf (stderr, "Hello, world!\n");

  if (min_msglen < P) min_msglen = P;

  /* Setup output filename */
  sprintf (outfile, "%s-%d.dat", bcast_algorithm (), P);

  if (rank == 0) {
    MPI_fprintf (stderr, "\n");
    MPI_fprintf (stderr, "Experimental parameters:\n");
    MPI_fprintf (stderr, "  Number of MPI processes: %d\n", P);
    MPI_fprintf (stderr, "  Smallest message tested: %d words (%d bytes)\n",
		 min_msglen, min_msglen * sizeof (int));
    MPI_fprintf (stderr, "  Largest message size tested: %d words (%.1f MiB)\n",
		 MAX_MSGLEN, (double)MAX_MSGLEN * sizeof (int) / 1024 / 1024);
    MPI_fprintf (stderr, "  Output file: %s\n", outfile);
    MPI_fprintf (stderr, "\n");
  }

  /* Open a file for writing results */
  MPI_fprintf (stderr, "Opening output file, %s...\n", outfile);
  if (rank == 0) {
    fp = fopen (outfile, "w");
    assert (fp != NULL);
    fprintf (fp, "#P\tBytes\tSeconds\tTrials\n");
  }

  /* Create a message buffer */
  MPI_fprintf (stderr, "Creating message buffers ...\n");
  msgbuf = (int *)malloc (MAX_MSGLEN * sizeof (int)); assert (msgbuf);

  /* Runs the asynchronous test-delay protocol */
  for (msglen = min_msglen; msglen <= MAX_MSGLEN; msglen <<= 1) {
    size_t trials = 3;
    double t_max = 0, t_elapsed;
    int i;

    /* Verify that the bcast protocol */
    MPI_fprintf (stderr, "Verifying protocol ... (message = %d words)\n", msglen);
    init_message (rank, msgbuf, msglen);
    bcast (msgbuf, msglen);
    i = test_message (rank, msgbuf, msglen);
    if (i != msglen) {
      MPI_fprintf (stderr, "*** ERROR *** msgbuf[%d] == %d\n", i, msgbuf[i]);
      assert (0);
    }
    MPI_fprintf (stderr, "==> Passed! (message = %d words)\n");

    MPI_fprintf (stderr, "Timing (%d words)...\n", msglen);
    do {
      double t_start;
      double t_elapsed;
      int k;
      MPI_Barrier (MPI_COMM_WORLD);
      t_start = MPI_Wtime ();
      for (k = 0; k < trials; ++k)
        bcast (msgbuf, msglen);
      t_elapsed = MPI_Wtime () - t_start;
      MPI_Barrier (MPI_COMM_WORLD);
      MPI_Allreduce (&t_elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
      if (t_max < MIN_TIME) trials <<= 1;
    } while (t_max < MIN_TIME);
    MPI_fprintf (stderr, "==> Done! (%d words, %g secs/trial)\n",
		 msglen, t_elapsed/trials);

    /* Write out the timing result */
    if (rank == 0) {
      const int bytes = msglen * sizeof (int);
      const double t_bcast = t_max / trials;
      MPI_fprintf (stdout, "%d\t%d\t%g\t%d\n", P, bytes, t_bcast, trials);
      fprintf (fp, "%d\t%d\t%g\t%d\n", P, bytes, t_bcast, trials);
      fflush (fp);
    }
  }

  MPI_fprintf (stderr, "Done! Cleaning up...\n");
  free (msgbuf);

  if (rank == 0)
    fclose (fp); /* Close output file */
  MPI_fprintf (stderr, "Shutting down MPI...\n");
  MPI_Finalize ();
  fprintf (stderr, "[rank %d of %d] End.\n", rank, P);
  return 0;
}

/* eof */
