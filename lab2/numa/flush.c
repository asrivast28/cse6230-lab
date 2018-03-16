/**
 *  \file flush.c
 *  \brief Implements a routine to flush the cache.
 */

#include <assert.h>

#define CACHE_BYTES (12 * 1024 * 1024)

#ifdef _OPENMP /********** OpenMP implementation **********/
#include <omp.h>
void flushBuffer (void)
{
  static size_t n_cache = 0; /* in 'size_t' words */
  static size_t* volatile * buffer = NULL;

  size_t n_threads = 0;
#pragma omp parallel default(none) shared(n_threads)
  {
#pragma omp master
      n_threads = omp_get_num_threads ();
  }


  if (!n_cache) {
    n_cache = 4 * CACHE_BYTES / sizeof (size_t) / n_threads;
    assert (n_cache);

    assert (!buffer);
    buffer = (size_t **)malloc (n_threads * sizeof (size_t *));
    assert (buffer);

#pragma omp parallel default(none) shared(n_threads,buffer,n_cache)
    {
      size_t tid = omp_get_thread_num ();
      buffer[tid] = (size_t *)malloc (n_cache * sizeof (size_t));
      assert (buffer[tid]);
    }
  }

#pragma omp parallel default(none) shared(buffer,n_cache)
  {
    const size_t tid = omp_get_thread_num ();
    size_t i;
    for (i = 0; i < n_cache; ++i)
      buffer[tid][i] = lrand48 ();
  }
}
#else  /********** sequential implementation **********/
void flushBuffer (void)
{
  static size_t n_cache = 0; /* in 'size_t' words */
  static size_t* volatile buffer = NULL;
  size_t i;
  if (!n_cache) {
    n_cache = 4 * CACHE_BYTES / sizeof (size_t);

    assert (!buffer);
    buffer = (size_t *)malloc (n_cache * sizeof (size_t));
    assert (buffer);
  }
  for (i = 0; i < n_cache; ++i)
    buffer[i] = lrand48 ();
}
#endif

// eof
