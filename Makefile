.DEFAULT_GOAL := all

CC = /usr/bin/gcc44
CXX = /usr/bin/g++44
CFLAGS = -std=gnu99
COPTFLAGS = -O2 -g
COMPFLAGS = -fopenmp
LDFLAGS =

CUDAROOT = /opt/cuda-4.2/cuda
NVCC = $(CUDAROOT)/bin/nvcc
NVCFLAGS = --compiler-bindir=$(CC)
NVCOPTFLAGS = $(COPTFLAGS)
NVLDFLAGS = --linker-options -rpath --linker-options $(CUDAROOT)/lib64
CUBLAS_LDFLAGS = -L$(CUDAROOT)/lib64 -Wl,-rpath -Wl,$(CUDAROOT)/lib64 -lcudart -lcublas

MPICC = mpicc
MPICFLAGS = $(CFLAGS)
MPICOPTFLAGS = $(COPTFLAGS) $(COMPFLAGS)
MPILDFLAGS =

HOST := $(shell hostname -f)
ifeq ($(HOST),daffy2)
  MKLDIR = /opt/intel/Compiler/11.1/080/Frameworks/mkl/lib/em64t
  BLAS_LDFLAGS = -L$(MKLDIR) -lmkl_intel_lp64 -lmkl_sequential
#else ifeq ($(HOST),jinx-login.cc.gatech.edu)
else
#  MKLDIR = /opt/intel/Compiler/11.1/059/mkl/lib/em64t
#  BLAS_LDFLAGS = -L$(MKLDIR) -Wl,-rpath -Wl,$(MKLDIR) -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
  MKLROOT = /nethome/rvuduc3/local/jinx/intel/composer_xe_2011_sp1.9.293/mkl
#  BLAS_LDFLAGS = -L$(MKLDIR)/lib/intel64 -Wl,-rpath -Wl,$(MKLDIR)/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
  BLAS_LDFLAGS =  -L$(MKLROOT)/lib/intel64 -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread -lm
endif

# ============================================================

TARGETS =
CLEANFILES =
DISTFILES =

COMMON_DEPS = timer.c timer.h Makefile
COMMON_DEPS += mpi_fprintf.h mpi_assert.h

# ============================================================

TARGETS += rev$(EXEEXT)
DISTFILES += rev.cu

rev$(EXEEXT): rev.cu $(COMMON_DEPS)
	$(NVCC) $(NVCFLAGS) $(NVCOPTFLAGS) -o $@ rev.cu \
	    $(NVLDFLAGS)

# ============================================================

TARGETS += mgpu-dma$(EXEEXT)
DISTFILES += mgpu-dma.cu

mgpu-dma$(EXEEXT): mgpu-dma.cu $(COMMON_DEPS)
	$(NVCC) $(NVCFLAGS) $(NVCOPTFLAGS) -o $@ mgpu-dma.cu \
	    $(NVLDFLAGS)

# ============================================================

TARGETS += mm1d-blas$(EXEEXT)
DISTFILES += mm1d.c mm-blas.c mm1d-blas.pbs

mm1d-blas$(EXEEXT): mm1d.c mm-blas.c $(COMMON_DEPS)
	$(MPICC) $(MPICFLAGS) $(MPICOPTFLAGS) -o $@ \
	    mm1d.c mm-blas.c \
	    $(BLAS_LDFLAGS)

# ============================================================

TARGETS += mm1d-cuda$(EXEEXT)
DISTFILES += mm1d.c mm-cuda.cu mm1d-cuda.pbs

mm1d-cuda$(EXEEXT): mm1d.c mm-cuda.o $(COMMON_DEPS)
	$(MPICC) $(MPICFLAGS) $(MPICOPTFLAGS) -o $@ \
	    mm1d.c mm-cuda.o \
	    $(CUBLAS_LDFLAGS)

soln-cuda: mm1d-cuda--soln$(EXEEXT)

mm1d-cuda--soln$(EXEEXT): mm1d.c mm-cuda--soln.o $(COMMON_DEPS)
	$(MPICC) $(MPICFLAGS) $(MPICOPTFLAGS) -o $@ \
	    mm1d.c mm-cuda--soln.o \
	    $(CUBLAS_LDFLAGS)

CLEANFILES += mm1d-cuda--soln$(EXEEXT) mm-cuda--soln.o

# ============================================================

%.o: %.cu $(COMMON_DEPS)
	$(NVCC) $(NVCFLAGS) $(NVCOPTFLAGS) -o $@ -c $<

# ============================================================

all: $(TARGETS)

dist: check-distroot $(DISTFILES) $(COMMON_DEPS)
	rm -rf $(DISTROOT)/
	mkdir -p $(DISTROOT)/
	cp $(DISTFILES) $(COMMON_DEPS) $(DISTROOT)/.
	tar cvf - $(DISTROOT)/* | gzip -9c > $(DISTROOT).tgz

check-distroot:
	@if test -z "$(DISTROOT)" ; then \
	    echo "*** To build a tarball (e.g., via $(MAKE) dist), please specify DISTROOT. ***" ; \
	    exit 1 ; \
	fi

clean:
	rm -f core *~
	rm -f mm1d-*.e[0-9][0-9]* *.o[0-9][0-9]*
	rm -f $(CLEANFILES) $(TARGETS)

# eof
