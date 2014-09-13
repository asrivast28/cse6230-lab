// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank.cc
 *  \brief Default (sequential) implementation.
 */

#include <strings.h> // for 'bzero'
#include "listrank.hh"

/* ====================================================================== */

rank_t *
createRanksBuffer (size_t n)
{
  rank_t* Rank = NULL;
  if (n) {
    Rank = new rank_t[n];
    assert (Rank);
    bzero (Rank, n * sizeof (rank_t));
  }
  return Rank;
}

void
releaseRanksBuffer (rank_t* Rank)
{
  if (Rank) delete[] Rank;
}

/* ====================================================================== */

void
computeListRanks (index_t head, const index_t* Next, rank_t* Rank)
{
  if (head == NIL) return; // empty list

  // Question: What does this loop do?
  index_t cur_node = head;
  rank_t count = 0;
  do {
    ++count;
    cur_node = Next[cur_node];
  } while (cur_node != NIL);

  // Question: What does this loop do?
  cur_node = head;
  do {
    Rank[cur_node] = --count;
    cur_node = Next[cur_node];
  } while (cur_node != NIL);
}

/* ====================================================================== */

void printListRanks (const string& tag,
                     index_t head, const index_t* Next,
                     const rank_t* Rank,
                     index_t truncate)
{
  index_t count = 0;

  cerr << "=== " << tag << " ===" << endl;
  index_t cur_node;

  cerr << "  Rank: [";
  cur_node = head;
  count = 0;
  if (cur_node == NIL)
    cerr << " (empty)";
  else
    do {
      if (!truncate || ++count <= truncate) {
        cerr << ' ' << Rank[cur_node];
        cur_node = Next[cur_node];
      } else {
        cerr << " ...";
        break;
      }
    } while (cur_node != NIL);
  cerr << " ]" << endl;

  cerr << "  Next: [";
  cur_node = head;
  count = 0;
  if (cur_node == NIL)
    cerr << " (empty)";
  else
    do {
      if (!truncate || ++count <= truncate) {
	cerr << ' ' << Next[cur_node];
	cur_node = Next[cur_node];
      } else {
	cerr << " ...";
	break;
      }
    } while (cur_node != NIL);
  cerr << " ]" << endl;
}

// eof
