// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file list.hh
 *  \brief Defines an interface for an "array pool" representation of
 *  a linked list.
 */

#if !defined (INC_LIST_HH)
#define INC_LIST_HH //!< list.hh included

#include <string>

typedef long index_t; //!< Node 'address': -1 (NIL), 0, 1, 2, 3, ...

#define NIL -1 //!< Index equivalent of a NULL pointer

/** Returns a newly allocated copy of an array */
index_t* duplicate (int n, const index_t* A);

/**
 *  Allocates a pool of 'next' pointers, and initializes into a single
 *  random linked list. The head is the first element ('next[0]') and
 *  the tail is the element whose next pointer is -1 (i.e., the
 *  element 'k' such that 'next[k] == NIL').
 */
index_t* createRandomList (int n);

/** Frees list array pool. */
void releaseListBuffer (index_t* Next);

#endif //!< list.hh included

// eof
