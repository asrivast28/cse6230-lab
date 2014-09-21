/**
 *  \file mgpu-dma.cu
 *  \brief Demo of how to use multiple GPUs.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <cuda.h>

#include "timer.c"

#define MINTIME 1 /*!< Minimum running time, in seconds */
#define MINTRIALS 3 /*!< Minimum number of timing trials, in seconds */
#define MAXVAL 1000 /*!< Maximum array value */
#define NUMEXP 10 /*!< Number of experimental trials */

#if CUDART_VERSION < 2020
#  error "*** Must have a CUDA 2.2 or greater. ***"
#endif

/**
 *  Performs a ping-pong through an intermediate buffer.
 */
static
void
pingpongBuffered (size_t n, int* A, int* B, int* tmpbuf)
{
  cudaMemcpy (tmpbuf, A, n * sizeof (int), cudaMemcpyDefault);
  cudaMemcpy (B, tmpbuf, n * sizeof (int), cudaMemcpyDefault);
  cudaMemcpy (tmpbuf, B, n * sizeof (int), cudaMemcpyDefault);
  cudaMemcpy (A, tmpbuf, n * sizeof (int), cudaMemcpyDefault);
  cudaDeviceSynchronize ();
}

/**
 *  Performs a "ping-pong," which is a round-trip pair of cudaMemcpys,
 *  the first from A to B and the second immediately returning from B
 *  to A.
 */
static
void
pingpongDirect (size_t n, int* A, int* B)
{
  cudaMemcpy (B, A, n * sizeof (int), cudaMemcpyDefault);
  cudaMemcpy (A, B, n * sizeof (int), cudaMemcpyDefault);
  cudaDeviceSynchronize ();
}

/**
 *  Performs a bunch of pingpongs (see above), and returns the average
 *  running time for each. "A bunch" means at least MINTRIALS
 *  pingpongs or MINTIME seconds, whichever occurs last. See also:
 *  pingpong().
 */
static long double benchmarkCopy (size_t n, int* A, int* B);

/**
 *  Performs a bunch of buffered ping-pongs, and returns the average
 *  running time for each. "A bunch" means at least MINTRIALS
 *  pingpongs or MINTIME seconds, whichever occurs last. See also:
 *  pingpongBuffered().
 */
static long double benchmarkBufferedCopy (size_t n, int* A, int* B, int* buf);

int
main (int argc, char* argv[])
{
  if (argc < 2) {
    fprintf (stderr, "usage: %s <n>\n", argv[0]);
    return -1;
  }

  long n_raw = atol (argv[1]);
  assert (n_raw > 0);
  const size_t n = (size_t)n_raw;
  const size_t n_bytes = n * sizeof (int);

  fprintf (stderr, "n = %lu (~ %.1f MiB)\n\n", n, (double)n_bytes/1024/1024);

  /* Timer setup */
  stopwatch_init ();

  /* Determine how many GPUs there are. Need at least two for this demo. */
  int gpu_count = 0;
  cudaGetDeviceCount (&gpu_count);
  fprintf (stderr, "Detected %d GPUs.\n", gpu_count);
  assert (gpu_count >= 2);

  /* Allocate arrays on each of two GPUs */
  int* A_gpu[2] = {NULL, NULL};
  cudaSetDevice (0);
  cudaMalloc ((void **)&(A_gpu[0]), n_bytes); assert (A_gpu[0]);
  cudaSetDevice (1);
  cudaMalloc ((void **)&(A_gpu[1]), n_bytes); assert (A_gpu[1]);

  /* Test 1: Ping-pong, using a temporary CPU buffer */
  fprintf (stderr, "Test 1: Buffering GPU-to-GPU copy using CPU memory...\n");
  int* A_cpu = (int *)malloc (n_bytes); assert (A_cpu);

  pingpongBuffered (n, A_gpu[0], A_gpu[1], A_cpu); /* warm-up */

  long double t_baseline = benchmarkBufferedCopy (n, A_gpu[0], A_gpu[1], A_cpu);
  printf ("==> GPU-to-GPU copy, buffered through CPU memory: %Lg seconds (%Lg GB/s)\n\n",
	  t_baseline, (long double)2e-9 * n_bytes / t_baseline);
  free (A_cpu);

  fprintf (stderr, "Test 2: Direct GPU-to-GPU copy...\n");
  pingpongDirect (n, A_gpu[0], A_gpu[1]);

  long double t_dma = benchmarkCopy (n, A_gpu[0], A_gpu[1]);
  printf ("==> Direct GPU-to-GPU copy: %Lg seconds (%Lg GB/s)\n\n",
	  t_dma, (long double)2e-9 * n_bytes / t_dma);

  cudaFree (A_gpu[0]);
  cudaFree (A_gpu[1]);
  return 0;
}

static
long double
benchmarkCopy (size_t n, int* A, int* B)
{
  struct stopwatch_t* timer = stopwatch_create (); assert (timer);

  long double t_elapsed = 0;
  size_t trials = 0;
  stopwatch_start (timer);
  while (trials < MINTRIALS || t_elapsed < MINTIME) {
    pingpongDirect (n, A, B);
    ++trials;
    t_elapsed = stopwatch_elapsed (timer);
  }
  t_elapsed = stopwatch_stop (timer);
  stopwatch_destroy (timer);
  fprintf (stderr, "  [%lu trials]\n", (unsigned long)trials);
  return t_elapsed / trials;
}

static
long double
benchmarkBufferedCopy (size_t n, int* A, int* B, int* tmpbuf)
{
  struct stopwatch_t* timer = stopwatch_create (); assert (timer);

  long double t_elapsed = 0;
  size_t trials = 0;
  stopwatch_start (timer);
  while (trials < MINTRIALS || t_elapsed < MINTIME) {
    pingpongBuffered (n, A, B, tmpbuf);
    ++trials;
    t_elapsed = stopwatch_elapsed (timer);
  }
  t_elapsed = stopwatch_stop (timer);
  stopwatch_destroy (timer);
  fprintf (stderr, "  [%lu trials]\n", (unsigned long)trials);
  return t_elapsed / trials;
}

/* eof */
