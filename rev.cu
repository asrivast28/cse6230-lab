/**
 *  \file rev.cu
 *  \brief CUDA unified virtual addressing benchmark example (reverse
 *  a list).
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <cuda.h>

#include "timer.c"

#define MINTIME 2.0 /*!< Minimum running time, in seconds */
#define MINTRIALS 3 /*!< Minimum number of timing trials, in seconds */
#define MAXVAL 1000 /*!< Maximum array value */

#if CUDART_VERSION < 2020
#  error "*** Must have a CUDA 2.2 or greater. ***"
#endif

/** Initialize A[i] = i % MAXVAL */
static void initArray (size_t n, int* A);

/** Checks that A[i] == i % MAXVAL */
static void verifyArray (size_t n, int* A);

/* ================================================== */

/** 
 *  Copies data from A_cpu (on the CPU) to A_gpu (on the GPU),
 *  reverses the data elements on the GPU, and copies the data back.
 */
__global__
void reverseArray (long n, int* A_gpu)
{
  long i = blockIdx.x * blockDim.x + threadIdx.x;
  const long n_half = n >> 1; /* floor (n / 2) */
  if (i < n_half) {
    const int i_pair = n - i - 1;
    int a = A_gpu[i];
    int b = A_gpu[i_pair];
    A_gpu[i] = b;
    A_gpu[i_pair] = a;
  }
}

/* ================================================== */

/** Benchmarks the reversal operation on unpinned memory. */
static
long double
benchmarkReverseWithCopies (size_t n, int* A_cpu)
{
  int* A_gpu = NULL;
  cudaMalloc ((void **)&A_gpu, n * sizeof (int)); assert (A_gpu);

  /* Do one test run */
  fprintf (stderr, "benchmarkReverseWithCopies: Testing...\n");
  const int BLOCKSIZE = 1024;
  const int NUMBLOCKS = (((n+1)/2) + BLOCKSIZE - 1) / BLOCKSIZE;
  initArray (n, A_cpu);
  cudaMemcpy (A_gpu, A_cpu, n * sizeof (int), cudaMemcpyDefault);
  reverseArray <<<NUMBLOCKS, BLOCKSIZE>>> (n, A_gpu);
  cudaMemcpy (A_cpu, A_gpu, n * sizeof (int), cudaMemcpyDefault);
  verifyArray (n, A_cpu);
  fprintf (stderr, "==> Passed!\n\n");

  /* Timing runs */
  fprintf (stderr, "benchmarkReverseWithCopies: Timing...\n");
  long double t_elapsed = 0;
  size_t trials = 0;
  stopwatch_init ();
  struct stopwatch_t* timer = stopwatch_create ();
  stopwatch_start (timer);
  while (trials < MINTRIALS || t_elapsed < MINTIME) {
    cudaMemcpy (A_gpu, A_cpu, n * sizeof (int), cudaMemcpyDefault);
    reverseArray <<<NUMBLOCKS, BLOCKSIZE>>> (n, A_gpu);
    cudaMemcpy (A_cpu, A_gpu, n * sizeof (int), cudaMemcpyDefault);
    cudaDeviceSynchronize ();
    ++trials;
    t_elapsed = stopwatch_elapsed (timer);
  }
  stopwatch_destroy (timer);
  fprintf (stderr, "==> %lu trials took %Lg seconds.\n", trials, t_elapsed);
  cudaFree (A_gpu);
  return t_elapsed / trials;
}

/* ================================================== */

/** Benchmarks the reversal operation on pinned memory. */
static
long double
benchmarkReverseWithoutCopies (size_t n, int* A_cpu_pinned)
{
  /* Do one test run */
  fprintf (stderr, "benchmarkReverseWithoutCopies: Testing...\n");
  const int BLOCKSIZE = 1024;
  const int NUMBLOCKS = (((n+1)/2) + BLOCKSIZE - 1) / BLOCKSIZE;
  initArray (n, A_cpu_pinned);
  reverseArray <<<NUMBLOCKS, BLOCKSIZE>>> (n, A_cpu_pinned);
  cudaDeviceSynchronize ();
  verifyArray (n, A_cpu_pinned);
  fprintf (stderr, "==> Passed!\n\n");

  /* Timing runs */
  fprintf (stderr, "benchmarkReverseWithoutCopies: Timing...\n");
  long double t_elapsed = 0;
  size_t trials = 0;
  stopwatch_init ();
  struct stopwatch_t* timer = stopwatch_create ();
  stopwatch_start (timer);
  while (trials < MINTRIALS || t_elapsed < MINTIME) {
    reverseArray <<<NUMBLOCKS, BLOCKSIZE>>> (n, A_cpu_pinned);
    cudaDeviceSynchronize ();
    ++trials;
    t_elapsed = stopwatch_elapsed (timer);
  }
  stopwatch_destroy (timer);
  fprintf (stderr, "==> %lu trials took %Lg seconds.\n", trials, t_elapsed);
  return t_elapsed / trials;
}

/* ================================================== */

#define TARGET(i) ((int)((i) % MAXVAL))

static
void
initArray (size_t n, int* A)
{
  for (size_t i = 0; i < n; ++i) {
    const int target = TARGET (i);
    A[i] = target;
  }
}

static
void
verifyArray (size_t n, int* A)
{
  for (size_t i = 0; i < n; ++i) {
    const int target = TARGET (n - i - 1);
    if (A[i] != target) {
      fprintf (stderr, "*** ERROR: Element A[%lu] == %d != %d! ***\n",
	       (unsigned long)i, A[i], target);
      assert (0);
    }
  }
}

/* ================================================== */

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

  fprintf (stderr, "n = %lu (~ %.1f MiB)\n", n, (double)n_bytes/1024/1024);

  fprintf (stderr, "Test 1: Using 'malloc' on CPU...\n");
  int* A_cpu = (int *)malloc (n_bytes);
  assert (A_cpu);
  long double t_baseline = benchmarkReverseWithCopies (n, A_cpu);
  printf ("==> Reversal with explicit copies: %Lg seconds (%Lg effective GB/s)\n\n",
	  t_baseline, (long double)2e-9 * n_bytes / t_baseline);
  free (A_cpu);

  fprintf (stderr, "Test 2: Using pinned cudaHostAlloc...\n");
  int* A_cpu_pinned = NULL;
  cudaHostAlloc ((void **)&A_cpu_pinned, n_bytes, cudaHostAllocMapped | cudaHostAllocPortable);
  assert (A_cpu_pinned);
  long double t_pinned = benchmarkReverseWithoutCopies (n, A_cpu_pinned);
  printf ("==> Reversal without explicit copies: %Lg seconds (%Lg effective GB/s)\n\n",
	  t_pinned, (long double)2e-9 * n_bytes / t_pinned);
  cudaFreeHost (A_cpu_pinned);

  return 0;
}

/* eof */
