/**
 *  \file triad.c
 *  \brief Demo of "triad", forall i: D[i] <- A[i] + b*C[i]
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <x86intrin.h>

#include "timer.h"
#include "flush.h"

typedef float real_t;

void triad__aligned (size_t n, real_t* D, const real_t* A, const real_t b, const real_t* C)
{
  __assume_aligned (D, 16);
  __assume_aligned (A, 16);
  __assume_aligned (C, 16);
  size_t i;
#pragma omp parallel for private(i) schedule(static)
  for (i = 0; i < n; ++i) {
    D[i] = A[i] + b*C[i];
  }
}

real_t* createArray__aligned (size_t n)
{
  real_t* A = NULL;
  posix_memalign (&((void *)A), 16, n * sizeof (size_t));
  assert (A);
  return A;
}

real_t createRandomValue (void)
{
  return (real_t)drand48 ();
}

void initRandom__aligned (size_t n, real_t* A)
{
  __assume_aligned (A, 16);
  size_t i;
  for (i = 0; i < n; ++i)
    A[i] = createRandomValue ();
}

static size_t get_size_t (const char* s)
{
  size_t n = 0;
  if (s) {
    long n_raw = atol (s);
    assert (n_raw >= 0);
    n = (size_t)n_raw;
  }
  return n;
}

int
main (int argc, char* argv[])
{
  size_t n;
  real_t* A;
  real_t b;
  real_t* C;
  real_t* D;

  size_t n_trials;
  size_t k;

  struct stopwatch_t* timer;

  if (argc < 3) {
    fprintf (stderr, "usage: %s <n> <trials>\n", argv[0]);
    return 1;
  }

  n = get_size_t (argv[1]);
  n_trials = get_size_t (argv[2]);

  A = createArray__aligned (n);
  C = createArray__aligned (n);
  D = createArray__aligned (n);

  stopwatch_init ();
  timer = stopwatch_create (); assert (timer);

  for (k = 0; k < n_trials; ++k) {
    long double t;

    fprintf (stderr, "... initializing ...\n");
    initRandom__aligned (n, A);
    b = createRandomValue ();
    initRandom__aligned (n, C);
    initRandom__aligned (n, D);
    flushBuffer ();

    fprintf (stderr, "... timing ...\n");
    stopwatch_start (timer);
    triad__aligned (n, D, A, b, C);
    t = stopwatch_stop (timer);

    printf ("%lu %lu %Lg %Lg\n",
	    (unsigned long)n,
	    (unsigned long)k,
	    t, (long double)n * 3e-9 * sizeof (real_t) / t);
  }

  stopwatch_destroy (timer);
  return 0;
}

/* eof */
