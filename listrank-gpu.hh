// -*- mode:c++; tab-width:2; indent-tabs-mode:nil;  -*-
/**
 *  \file listrank-gpu.hh
 *  \brief Interface for a GPU-accelerated list ranking kernel.
 */

#if !defined (INC_LISTRANK_MT_HH)
#define INC_LISTRANK_MT_HH //!< listrank-mt.hh included

#include "listrank.hh"

/** GPU-accelerated implementation of rankList__seq (); see 'listrank.hh' */
extern "C" void rankList__gpu (int n, rank_t* Rank, const index_t* Next, index_t head);

#endif // !defined (INC_LISTRANK_GPU_HH)

// eof
