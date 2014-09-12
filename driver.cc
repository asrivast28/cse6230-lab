// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file driver.cc
 *  \brief List ranking benchmark driver.
 */

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <string>
#include "timer.h"

#include "listrank-seq.hh"

using namespace std;

/* ====================================================================== */

static void
check (int n, const rank_t* R, const rank_t* R_true)
{
  for (int i = 0; i < n; ++i)
    if (R[i] != R_true[i]) {
      cerr << "*** ERROR: *** [" << i << " ] Rank " << R[i] << " != " << R_true[i] << endl;
      assert (false);
    }
  cerr << "    (OK!)" << endl;
}

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

  for (int trial = 0; trial < NTRIALS; ++trial) {
    cerr << endl << "... [" << trial << "] creating node pool ..." << endl;
    index_t* Next = createRandomList (N);

    cerr << endl << "... running sequential algorithm ..." << endl;
    rank_t* Rank_seq = new size_t[N]; assert (Rank_seq);
    bzero (Rank_seq, N * sizeof (rank_t));
    printList ("Sequential: before", Rank_seq, Next);
    stopwatch_start (timer);
    rankList__seq (N, Rank_seq, Next, 0 /* head */);
    long double t_seq = stopwatch_stop (timer); /* seconds */
    long double bw_seq = estimateBandwidth (N, t_seq) * 1e-9; /* GB/s */
    cerr << "    (Done: " << t_seq << " sec, " << bw_seq << " GB/s.)" << endl;
    printList ("Sequential: after", Rank_seq, Next);

    cerr << endl << "... running parallel algorithm ..." << endl;
    rank_t* Rank_par = new size_t[N]; assert (Rank_par);
    bzero (Rank_par, N * sizeof (rank_t));
    printList ("Parallel: before", Rank_par, Next);
    stopwatch_start (timer);
#if 0
    rankList__par (N, Rank_par, Next);
#endif
    long double t_par = stopwatch_stop (timer); /* seconds */
    long double bw_par = estimateBandwidth (N, t_par) * 1e-9; /* GB/s */
    cerr << "    (Done: " << t_par << " sec, " << bw_par << " GB/s.)" << endl;
    printList ("Parallel: after", Rank_par, Next);

    cout << trial
         << ' ' << N << ' ' << sizeof (index_t) << ' ' << sizeof (rank_t)
         << ' ' << t_seq << ' ' << bw_seq << ' ' << t_par << ' ' << bw_par
         << endl;

    cerr << endl << "... checking the answer ..." << endl;
    check (N, Rank_par, Rank_seq);

    delete[] Rank_par;
    delete[] Rank_seq;
    delete[] Next;
  }

  stopwatch_destroy (timer);
  return 0;
}

// eof
