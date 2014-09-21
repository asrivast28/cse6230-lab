#if !defined (INC_MPI_FPRINTF_H)
#define INC_MPI_FPRINTF_H

static
void
MPI_fprintf_debug (const char* source, size_t line,
		   FILE* fp, const char* fmt, ...)
{
  va_list args;

  int rank = 0;
  int np = 0;
  char hostname[MPI_MAX_PROCESSOR_NAME+1];
  int namelen = 0;

  va_start (args, fmt);

  MPI_Comm_rank (MPI_COMM_WORLD, &rank); /* Get process id */
  MPI_Comm_size (MPI_COMM_WORLD, &np);	 /* Get number of processes */
  MPI_Get_processor_name (hostname, &namelen); /* Get hostname of node */

  fprintf (fp, "[%s:rank %d of %d -- %s:%lu] ",
	   hostname, rank, np, source, (unsigned long)line);
  vfprintf (fp, fmt, args);
  fflush (fp);

  va_end (args);
}

/* http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 * #define eprintf(format, ...) fprintf (stderr, format, ##__VA_ARGS__)
 */
#define MPI_fprintf(fp, fmt, ...)		\
  MPI_fprintf_debug (__FILE__, __LINE__, (fp), (fmt), ##__VA_ARGS__)

#endif

/* eof */
