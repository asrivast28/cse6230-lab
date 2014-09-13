// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file list.cc
 *  \brief Implements the list.hh interface.
 */

#include <cassert>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <iostream>

#include "list.hh"

using namespace std;

/* ====================================================================== */

index_t *
duplicate (size_t n, const index_t* A)
{
  index_t* B = new index_t[n]; assert (B);
  memcpy (B, A, n * sizeof (index_t));
  return B;
}

void
releaseListBuffer (index_t* Next)
{
  if (Next) delete[] Next;
}

/** Generates a uniform random permutation of an array */
static void
shuffle (size_t n, index_t* A)
{
  // Implements the Fisher-Yates (Knuth) algorithm:
  // http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
  for (size_t i = 0; i < (n-1); ++i)
    swap (A[i], A[i+1+(lrand48 () % (n-i-1))]);
}

/* ====================================================================== */

index_t *
createRandomList (size_t n)
{
  // Create an initial linked list where each node i points to i+1.
  index_t* Init = new index_t[n]; assert (Init);
  for (size_t i = 0; i < n; ++i)
    Init[i] = i + 1;
  Init[n-1] = NIL; // "NULL" pointer

  // Remap node i > 0 to position AddrMap[i].
  index_t* AddrMap = new index_t[n]; assert (AddrMap);
  for (size_t i = 0; i < n; ++i)
    AddrMap[i] = i;
  shuffle (n-1, AddrMap+1);

  // Create final list
  index_t* Next = new index_t[n]; assert (Next);
  for (size_t i = 0; i < n; ++i)
    Next[AddrMap[i]] = Init[i] > 0 ? AddrMap[Init[i]] : NIL;

  delete[] AddrMap;
  delete[] Init;
  return Next;
}

// eof
