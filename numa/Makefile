.DEFAULT_GOAL := all

EXEEXT =

CC = icc
CFLAGS =
COPTFLAGS = -O2 -g
COMPFLAGS = -openmp

HDRS = timer.h flush.h
SRCS = triad.c $(HDRS:.h=.c)
TARGETS = triad$(EXEEXT)

all: $(TARGETS)
	@echo "=== done ==="

triad$(EXEEXT): $(SRCS) $(HDRS) Makefile
	$(CC) $(CFLAGS) $(COPTFLAGS) $(COMPFLAGS) -o $@ $(SRCS)

clean:
	rm -f core *~ *.o

# eof
