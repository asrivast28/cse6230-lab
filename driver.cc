// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file driver.cc
 *  \brief List ranking benchmark driver.
 */

#include <cassert>
#include <cstdlib>
#include <strings.h> // for bzero

#include <algorithm>
#include <iostream>
#include <string>

#include "timer.h"

#include "list.hh"
#include "listrank.hh"
#include "listrank-par.hh"

using namespace std;

/* ====================================================================== */

/** Compares two rank buffers and aborts the program if they are unequal. */
static void
assertListRanksMatch (size_t n, const rank_t* R, const rank_t* R_true)
{
  for (size_t i = 0; i < n; ++i)
    if (R[i] != R_true[i]) {
      cerr << "*** ERROR: *** [" << i << " ] Rank " << R[i] << " != " << R_true[i] << endl;
      assert (false);
    }
  cerr << "    (OK!)" << endl;
}

/**
 *  Performs a quick test of the parallel list ranking implementation
 *  against a trusted sequential implementation, for a problem of size
 *  N; aborts the program if the check fails.
 */
static void
checkParallelListRanker (size_t N)
{
  // Use sequential implementation to compute the 'trusted' ranks
  index_t* Next = createRandomList (N);
  rank_t* Rank_true = createRanksBuffer (N);
  computeListRanks (0, Next, Rank_true);

  // Run the parallel implementation
  ParRankedList_t* rankedList = setupRanks__par (N, Next);
  computeListRanks__par (rankedList);
  const rank_t* Rank_par = getRanks__par (rankedList);

  // Check the answer
  assertListRanksMatch (N, Rank_par, Rank_true);

  releaseRanks__par (rankedList);
  releaseRanksBuffer (Rank_true);
  releaseListBuffer (Next);
}

/* ====================================================================== */

/**
 *  Given a list size 'n' and the execution time 't' in seconds,
 *  returns an estimate of the effective bandwidth (in bytes per
 *  second) of list ranking.
 */
static long double
estimateBandwidth (size_t n, long double t)
{
  return (long double)n * (2*sizeof (index_t) + sizeof (rank_t)) / t;
}

/**
 *  Computes the min, max, mean, and median values of an array
 *  T[0:n-1] of 'n' long doubles.
 */
static void
getStats (long double* T, size_t n,
          long double& t_min, long double& t_max,
          long double& t_mean, long double& t_median)
{
  assert (n > 0);
  t_min = T[0];
  t_max = T[0];
  t_mean = T[0];
  t_median = T[0];
  for (size_t i = 1; i < n; ++i) {
    if (T[i] < t_min) t_min = T[i];
    if (T[i] < t_max) t_max = T[i];
    t_mean += T[i];
  }
  t_mean /= n;
  t_median = selectMedian (T, n);
}

/**
 *  Benchmarks the sequential list ranking implementation on the given
 *  linked list. , and returns its running time, in seconds.
 */
static void
benchmarkSequential (size_t N, size_t num_trials, struct stopwatch_t* timer)
{
  if (!num_trials) return;

  assert (timer);
  assert ((Next && Rank) || !N);

  cerr << endl << "... benchmarking the sequential algorithm ..." << endl;

  long double* Times = new long double[num_trials]; assert (times);

  for (size_t trial = 0; trial < num_trials; ++trial) {
    index_t* Next = createRandomList (N);
    rank_t* Rank_true = createRanksBuffer (N);
    stopwatch_start (timer);
    computeListRanks (0, Next, Rank_true);
    Times[trial] = stopwatch_stop (timer);
  }

  long double t_min, t_max, t_median, t_mean;
  cout << "SEQ" << ',' << N << ',' << num_trials
       << ',' << estimateBandwidth (N, t_median)
       << ',' << t_min << ',' << t_median << ',' << t_max << ',' << t_mean
       << endl;

  delete[] Times;
}

/* ====================================================================== */

#if 0
/**
 *  Benchmarks the sequential list ranking implementation on the given
 *  linked list, and returns its running time, in seconds.
 */
static long double
benchmarkParallel (int N, index_t* Next, struct stopwatch_t* timer)
{
    cerr << endl << "... running parallel algorithm ..." << endl;
    rank_t* Rank_par = new size_t[N]; assert (Rank_par);
    bzero (Rank_par, N * sizeof (rank_t));
    printList ("Parallel: before", Rank_par, Next);
    stopwatch_start (timer);
#if 0
    rankList__par (N, Rank_par, Next);
#endif
}
#endif

static void
benchmarkListRankers (size_t n, size_t num_trials, struct stopwatch_t* timer)
{
  benchmarkSequential (n, num_trials, timer);
}

/* ====================================================================== */

int
main (int argc, char* argv[])
{
  if (argc != 3) {
    cerr << endl << "usage: " << argv[0] << " <n> <trials>\n" << endl;
    return -1;
  }

  int N = atoi (argv[1]); assert (N > 0);
  int NTRIALS = atoi (argv[2]); assert (NTRIALS > 0);

  cerr << endl
       << "N: " << N << endl
       << "Node size: " << sizeof (index_t) << " + " << sizeof (rank_t) << " bytes" << endl
       << "Trials: " << NTRIALS << endl
       << endl;

  stopwatch_init ();
  struct stopwatch_t* timer = stopwatch_create ();
  assert (timer);

  checkParallelListRanker (N);
  benchmarkListRankers (N, NTRIALS, timer);

  stopwatch_destroy (timer);
  return 0;
}

// eof
