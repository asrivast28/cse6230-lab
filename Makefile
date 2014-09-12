.DEFAULT_GOAL := all

EXEEXT =
TARGETS = listrank$(EXEEXT)

HDRS =
HDRS += listrank.hh
SRCS += listrank.cc
OBJS += listrank.o

HDRS += listrank-mt.hh
SRCS += listrank-mt.cc
OBJS += listrank-mt.o

#HDRS += listrank-gpu.hh
#SRCS += listrank-gpu.cu
#OBJS += listrank-gpu.o

CC = icpc
CFLAGS = -O3 -g
LDLFLAGS =

all: $(TARGETS)

listrank$(EXEEXT): $(HDRS) $(SRCS) Makefile
	$(CC) $(CFLAGS) $(COPTFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	rm -f $(OBJS) *~ core $(TARGETS)

# eof
