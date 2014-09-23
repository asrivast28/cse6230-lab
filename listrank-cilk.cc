// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank-cilk.cc
 *
 *  \brief Implement the 'listrank-par.hh' interface using Cilk Plus.
 */

#include <cassert>
#include <cmath>
#include <cstring>

#include <algorithm>
#include <iostream>

#include "listrank-par.hh"

#include <cilk/reducer_opor.h>

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
  index_t* N[2]; 
  const rank_t* Rank;
  rank_t* R[2];
};

// ============================================================

ParRankedList_t *
setupRanks__par (size_t n, const index_t* Next)
{
  ParRankedList_t* L = new ParRankedList_t;
  assert (L);

  L->n = n;

  L->Next = Next;
  L->N[0] = duplicate (n, Next);
  L->N[1] = new index_t[n]; assert (L->N[1]);

  for (size_t i = 0; i < 2; ++i) {
    L->R[i] = createRanksBuffer (n);
  }

  L->Rank = L->R[0];

  return L;
}

void releaseRanks__par (ParRankedList_t* L)
{
  if (L) {
    for (size_t i = 0; i < 2; ++i) { 
      releaseRanksBuffer (L->R[i]);
      releaseListBuffer (L->N[i]);
    }

    L->Rank = NULL;
    L->Next = NULL;
  }
}

// ============================================================

const rank_t *
getRanks__par (const ParRankedList_t* L)
{
  return L->Rank;
}

template <class pointer_t>
static void
swapPointers (pointer_t* &ptr1, pointer_t* &ptr2)
{
  pointer_t* ptrTemp = ptr1;
  ptr1 = ptr2;
  ptr2 = ptrTemp;
}
// ============================================================

static void
computeListRanks__cilk__ (ParRankedList_t* L)
{
  size_t n = L->n;
  if (n == 0) return; // empty pool
  assert (L->Next);
  assert (L->Rank);

  index_t* N_cur = L->N[0];
  index_t* N_next = L->N[1];

  rank_t* R_cur = L->R[0];
  rank_t* R_next = L->R[1];

  // Initial values on which we will perform the list-based 'scan' /
  // 'prefix sum'
  _Cilk_for (size_t i = 0; i < n; ++i) {
    R_cur[i] = (N_cur[i] == NIL) ? 0 : 1;
  }

  size_t maxIterations = static_cast<size_t>(ceil(log2(static_cast<double>(n)))); 

  for (size_t j = 0; j < maxIterations; ++j) {
    _Cilk_for (size_t i = 0; i < n; ++i) {
      if (N_cur[i] != NIL) {
        R_next[i] = R_cur[i] + R_cur[N_cur[i]]; 
        N_next[i] = N_cur[N_cur[i]];
      }
      else {
        R_next[i] = R_cur[i];
        N_next[i] = NIL;
      }
    }

    swapPointers<index_t> (N_cur, N_next);
    swapPointers<rank_t> (R_cur, R_next);
  }

  L->Rank = R_cur;
}

void
computeListRanks__par (ParRankedList_t* L)
{
  assert (L != NULL);
  computeListRanks__cilk__ (L);
}

// eof
