// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank.cc
 *  \brief Default (sequential) implementation.
 */

#include "listrank.hh"

void
rankList (int n, rank_t* Rank, const index_t* Next, index_t head)
{
  if (n == 0 || head == NIL) return; // pool or list are empty

  // What does this loop do?
  index_t cur_node = head;
  rank_t count = 0;
  do {
    ++count;
    cur_node = Next[cur_node];
  } while (cur_node != NIL);

  // What does this loop do?
  cur_node = head;
  do {
    Rank[cur_node] = --count;
    cur_node = Next[cur_node];
  } while (cur_node != NIL);
}

/* ====================================================================== */

// eof
