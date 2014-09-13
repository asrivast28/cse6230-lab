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
#include "quickselect.hh"

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
    if (T[i] > t_max) t_max = T[i];
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

  cerr << endl << "... benchmarking the sequential algorithm ..." << endl;

  long double* Times = new long double[num_trials]; assert (Times);

  for (size_t trial = 0; trial < num_trials; ++trial) {
    index_t* Next = createRandomList (N);
    rank_t* Rank = createRanksBuffer (N);
    stopwatch_start (timer);
    computeListRanks (0, Next, Rank);
    Times[trial] = stopwatch_stop (timer);
    releaseListBuffer (Next);
    releaseRanksBuffer (Rank);
  }

  long double t_min, t_max, t_median, t_mean;
  getStats (Times, num_trials, t_min, t_max, t_mean, t_median);
  cout << "SEQ" << ',' << N << ',' << num_trials
       << ',' << estimateBandwidth (N, t_median)*1e-9
       << ',' << t_min << ',' << t_median << ',' << t_max << ',' << t_mean
       << endl;

  delete[] Times;
}

/* ====================================================================== */

/**
 *  Benchmarks the sequential list ranking implementation on the given
 *  linked list, and returns its running time, in seconds.
 */
static void
benchmarkParallel (size_t N, size_t num_trials, struct stopwatch_t* timer)
{
  if (!num_trials) return;

  assert (timer);

  cerr << endl << "... benchmarking the parallel algorithm ..." << endl;

  long double* T_pre = new long double[num_trials]; assert (T_pre);
  long double* T_rank = new long double[num_trials]; assert (T_rank);
  long double* T_post = new long double[num_trials]; assert (T_post);

  for (size_t trial = 0; trial < num_trials; ++trial) {
    index_t* Next = createRandomList (N);

    stopwatch_start (timer);
    ParRankedList_t* rankedList = setupRanks__par (N, Next);
    T_pre[trial] = stopwatch_stop (timer);

    stopwatch_start (timer);
    computeListRanks__par (rankedList);
    T_rank[trial] = stopwatch_stop (timer);

    stopwatch_start (timer);
    const rank_t* Rank = getRanks__par (rankedList);
    T_post[trial] = stopwatch_stop (timer);

    releaseRanks__par (rankedList);
    releaseListBuffer (Next);
  }

  long double t_min, t_max, t_median, t_mean;
  getStats (T_rank, num_trials, t_min, t_max, t_mean, t_median);
  const char* name = getImplName__par ();
  cout << name << ',' << N << ',' << num_trials
       << ',' << estimateBandwidth (N, t_median)*1e-9
       << ',' << t_min << ',' << t_median << ',' << t_max << ',' << t_mean;
  getStats (T_pre, num_trials, t_min, t_max, t_mean, t_median);
  cout << ',' << t_min << ',' << t_median << ',' << t_max << ',' << t_mean;
  getStats (T_post, num_trials, t_min, t_max, t_mean, t_median);
  cout << ',' << t_min << ',' << t_median << ',' << t_max << ',' << t_mean;
  cout << endl;

  delete[] T_post;
  delete[] T_rank;
  delete[] T_pre;
}

static void
benchmarkListRankers (size_t n, size_t num_trials, struct stopwatch_t* timer)
{
  benchmarkSequential (n, num_trials, timer);
  benchmarkParallel (n, num_trials, timer);
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
