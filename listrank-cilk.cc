// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank-cilk.cc
 *
 *  \brief Implement the 'listrank-par.hh' interface using Cilk Plus.
 */

#include <cassert>
#include <cstring>

#include <algorithm>
#include <iostream>

#include "listrank-par.hh"

using namespace std;

// ============================================================
const char *
getImplName__par (void)
{
  return "CILK";
}

// ============================================================

struct ParRankedList_t__
{
  size_t n;
  const index_t* Next;
  rank_t* Rank;
};

// ============================================================

ParRankedList_t *
setupRanks__par (size_t n, const index_t* Next)
{
  ParRankedList_t* L = new ParRankedList_t;
  assert (L);

  L->n = n;
  L->Next = Next;
  L->Rank = createRanksBuffer (n);

  return L;
}

void releaseRanks__par (ParRankedList_t* L)
{
  if (L) {
    releaseRanksBuffer (L->Rank);
  }
}

// ============================================================

const rank_t *
getRanks__par (const ParRankedList_t* L)
{
  return L->Rank;
}

// ============================================================

static void
computeListRanks__cilk__ (size_t n, const index_t* Next, rank_t* Rank)
{
  if (n == 0) return; // empty pool
  assert (Next);
  assert (Rank);

  // Initial values on which we will perform the list-based 'scan' /
  // 'prefix sum'
  _Cilk_for (size_t i = 0; i < n; ++i)
    Rank[i] = (Next[i] == NIL) ? 0 : 1;

  //------------------------------------------------------------
  //
  // ... YOUR CODE GOES HERE ...
  //
  // (you may also modify any of the preceding code if you wish)
  //
  //#include "soln--cilk.cc" // Instructor's solution: none for you!
  //------------------------------------------------------------
}

void
computeListRanks__par (ParRankedList_t* L)
{
  assert (L != NULL);
  computeListRanks__cilk__ (L->n, L->Next, L->Rank);
}

// eof
