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

static void
verifyParallelListRanker (size_t N)
{
  // Use sequential implementation to compute the 'trusted' ranks
  index_t* Next = createRandomList (N);
  rank_t* Rank_true = createRanksBuffer (N);
  computeListRanks (0, Next, Rank_true);

  // Run the parallel implementation
  ParRankedList_t* rankedList = setupRanks__par (N, Next);
  computeListRanks__par (rankedList);
  rank_t* Rank_par = getRanks__par (rankedList, Rank_par);

  // Check the answer
  assertListRanksMatch (N, Rank_par, Rank_true);

  releaseRanks__par (rankedList);
  releaseRanksBuffer (Rank_true);
  releaseListBuffer (Next);
}

/* ====================================================================== */

#if 0

/**
 *  Given a list size 'n' and the execution time 't' in seconds,
 *  returns an estimate of the effective bandwidth (in bytes per
 *  second) of list ranking.
 */
static long double
estimateBandwidth (int n, long double t)
{
  return (long double)n * (2*sizeof (index_t) + sizeof (rank_t)) / t;
}

/* ====================================================================== */

/**
 *  Benchmarks the sequential list ranking implementation on the given
 *  linked list. , and returns its running time, in seconds.
 */
static long double
benchmarkSequential (size_t N, const index_t* Next, rank_t* Rank,
                     struct stopwatch_t* timer)
{
  long double t = -1;

  assert (Next || !N);
  cerr << endl << "... running sequential algorithm ..." << endl;
  bzero (Rank, N * sizeof (rank_t));
  printListRanks ("Sequential: before", Rank, Next);

  if (timer) stopwatch_start (timer);
  computeListRanks (N, Rank, Next, 0 /* head */);
  if (timer) t = stopwatch_stop (timer);

  printListRanks ("Sequential: before", Rank, Next);
  return t;
}

/* ====================================================================== */

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

/* ====================================================================== */
#endif // #if 0

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

  verifyParallelListRanker (N);
#if 0
  benchmarkListRankers (N, NTRIALS, timer);

  long double* T_seq = new long double[NTRIALS]; assert (T_seq);
  long double* T_par = new long double[NTRIALS]; assert (T_par);
  long double* T_par_pre = new long double[NTRIALS]; assert (T_par_pre);
  long double* T_par_post = new long double[NTRIALS]; assert (T_par_post);
  for (int trial = 0; trial < NTRIALS; ++trial) {
    cerr << endl << "... [" << trial << "] creating node pool ..." << endl;
    index_t* Next = createRandomList (N);
    rank_t* Rank_seq = new rank_t[N]; assert (Rank_seq);

    T_seq[trial] = benchmarkSequential (N, Next, Rank_seq, timer);
    long double bw_seq = estimateBandwidth (N, t_seq) * 1e-9; /* GB/s */
    cerr << "    (Done: " << t_seq << " sec, " << bw_seq << " GB/s.)" << endl;
    printListRanks ("Sequential: after", Rank_seq, Next);

    long double t_par = -1;
    long double t_par_pre = -1;
    long double t_par_post = -1;
    benchmarkParallel (N, Next, t_par_pre, t_par, t_par_post);
    long double bw_par = estimateBandwidth (N, t_par) * 1e-9; /* GB/s */
    cerr << "    (Done: " << t_par << " sec, " << bw_par << " GB/s.)" << endl;
    printList ("Parallel: after", Rank_par, Next);

    cout << trial
         << ' ' << N << ' ' << sizeof (index_t) << ' ' << sizeof (rank_t)
         << ' ' << t_seq << ' ' << bw_seq << ' ' << t_par << ' ' << bw_par
         << endl;

    cerr << endl << "... checking the answer ..." << endl;
    assertListRanksMatch (N, Rank_par, Rank_seq);

    delete[] Rank_par;
    delete[] Rank_seq;
    delete[] Next;
  }
#endif

  stopwatch_destroy (timer);
  return 0;
}

// eof
