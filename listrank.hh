// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank.hh
 *  \brief Defines an "API" for list ranking.
 */

#if !defined (INC_LISTRANK_HH)
#define INC_LISTRANK_HH //!< list.hh included

#include "list.hh"

typedef unsigned long rank_t; //!< Rank value: 0, 1, 2, 3, ...

/** Returns new space for storing ranks. */
rank_t* createRanksBuffer (size_t n);

/** Frees rank buffer space. */
void releaseRanksBuffer (rank_t* Rank);

/**
 *  Given a linked list represented as an array pool, this routine
 *  computes 'Rank[i]' to be the distance to the tail of each node
 *  'i'. The head of the list is at position 'head', i.e., the node
 *  represented by 'Rank[head]' and 'Next[head]'. 'head' may be 'NIL'
 *  if the list is empty.
 */
void computeListRanks (index_t head, const index_t* Next, rank_t* Rank);

/**
 *  (Debugging) Prints the contents of a ranked list, truncating the
 *  output after 'truncate' elements.
 */
void printListRanks (const std::string& tag,
                     index_t head, const index_t* Next,
                     const rank_t* Rank,
                     index_t truncate=16);

#endif // !defined (INC_LISTRANK_HH)

// eof
