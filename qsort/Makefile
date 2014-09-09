CC = icpc
CFLAGS =
COPTFLAGS = -O3 -g -fopenmp
LDFLAGS =

qsort-omp: driver.o sort.o parallel-qsort--omp.o
	$(CC) $(COPTFLAGS) -o $@ $^

%.o: %.cc
	$(CC) $(CFLAGS) $(COPTFLAGS) -o $@ -c $<

clean:
	rm -f core *.o *~

# eof
