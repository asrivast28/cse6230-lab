// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank-mt.cc
 *  \brief Implement your Cilk Plus list ranking code here.
 */

#include <cassert>
#include <cstring>
#include "listrank-mt.hh"

static void
rankList__cilk__ (int n, rank_t* Rank, const index_t* Next)
{
  if (n == 0) return; // empty pool

  // Initial values on which we will perform the list-based 'scan' /
  // 'prefix sum'
  _Cilk_for (int i = 0; i < n; ++i)
    Rank[i] = (Next[i] == NIL) ? 0 : 1;

  // To help implement synchronization, it may be helpful have
  // additional buffers. Here is some example code:
  rank_t* Rank_cur = Rank;
  index_t* Next_cur = duplicate (n, Next); // function we've provided
  rank_t* Rank_next = new rank_t[n]; assert (Rank_next);
  index_t* Next_next = duplicate (n, Next);

  //------------------------------------------------------------
  //
  // ... YOUR CODE GOES HERE ...
  //
  // (you may also modify the preceding code if you wish)
  //
  //------------------------------------------------------------

  // If you use extra buffers, be sure to clean-up after yourself,
  // e.g.,
  if (Rank != Rank_cur) {
    memcpy (Rank, Rank_cur, n * sizeof (rank_t));
    delete[] Rank_cur;
  } else { delete[] Rank_next; }
  delete[] Next_next;
  delete[] Next_cur;
}

void
rankList__mt (int n, rank_t* Rank, const index_t* Next)
{
  rankList__cilk__ (n, Rank, Next);
}

// eof
