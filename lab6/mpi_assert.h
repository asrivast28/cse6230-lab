#if !defined (INC_MPI_ASSERT_H)
#define INC_MPI_ASSERT_H

#include <assert.h>
#include <stdlib.h>
#include <mpi.h>

#define MPI_Assert(comm, cond)  MPI_Assert__ ((comm), __FILE__, __LINE__, (cond), #cond)

static
void
MPI_Assert__ (MPI_Comm comm, const char* file, size_t line, int cond, const char* cond_msg)
{
#if !defined (NDEBUG)
  if (!cond) {
    MPI_fprintf_debug (comm, file, line, stderr, "ASSERTION FAILED: '%s' is false\n", cond_msg);
    MPI_Abort (comm, cond);
  }
#endif
}

#endif
