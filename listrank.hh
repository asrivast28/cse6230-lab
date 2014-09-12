// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank.hh
 *  \brief Defines an "API" for list ranking.
 */

#if !defined (INC_LISTRANK_HH)
#define INC_LISTRANK_HH //!< list.hh included

#include "list.hh"

/**
 *  Given a linked list represented as an array pool, this routine
 *  computes 'Rank[i]' to be the distance to the tail of each node
 *  'i'. The head of the list is at position 'head', i.e., the node
 *  represented by 'Rank[head]' and 'Next[head]'. 'head' may be 'NIL'
 *  if the list is empty.
 */
extern "C" void rankList (int n, rank_t* Rank, const index_t* Next, index_t head);

#endif // !defined (INC_LISTRANK_HH)

// eof
