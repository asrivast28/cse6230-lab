.DEFAULT_GOAL := all

MPICC = mpicc
MPICFLAGS = -std=c99
MPICOPTFLAGS = -O2 -g
MPILDFLAGS =

EXEEXT =
DISTFILES =
CLEANFILES =

BINS = serial$(EXEEXT) tree$(EXEEXT) bigvec$(EXEEXT)

all: $(BINS)

help:
	@echo "=================================================="
	@echo "usage: $(MAKE) [cmd]"
	@echo ""
	@echo "Possible [cmd] values:"
	@echo ""
	@echo "  (empty)"
	@echo "     Build all binary targets -- $(BINS)"
	@echo ""
	@echo "  pbs ALG=<alg> P=<num-procs>"
	@echo "     Submit a Torque/PBS job for algorithm <alg> running with <num-procs> MPI tasks."
	@echo ""
	@echo "  plot-alg ALG=<alg>"
	@echo "     Run a gnuplot script to display just the data for <alg> at different MPI process counts."
	@echo ""
	@echo "  plot-all P=<num-procs>"
	@echo "     Run a gnuplot script to display the data for all algorithms at MPI process count=8."
	@echo ""
	@echo "Possible values of <alg>: {serial, tree, bigvec}"
	@echo "=================================================="

SRCS_COMMON = driver.c
DEPS_COMMON = mpi_fprintf.h Makefile

DISTFILES += $(SRCS_COMMON) $(DEPS_COMMON)

serial$(EXEEXT): serial.c $(SRCS_COMMON) $(DEPS_COMMON)
	$(MPICC) $(MPICFLAGS) $(MPICOPTFLAGS) \
	    -o $@ serial.c $(SRCS_COMMON) \
	    $(MPILDFLAGS)

CLEANFILES += serial$(EXEEXT)
DISTFILES += serial.c

tree$(EXEEXT): tree.c $(SRCS_COMMON) $(DEPS_COMMON)
	$(MPICC) $(MPICFLAGS) $(MPICOPTFLAGS) \
	    -o $@ tree.c $(SRCS_COMMON) \
	    $(MPILDFLAGS)

CLEANFILES += tree$(EXEEXT)
DISTFILES += tree.c

bigvec$(EXEEXT): bigvec.c $(SRCS_COMMON) $(DEPS_COMMON)
	$(MPICC) $(MPICFLAGS) $(MPICOPTFLAGS) \
	    -o $@ bigvec.c $(SRCS_COMMON) \
	    $(MPILDFLAGS)

CLEANFILES += bigvec$(EXEEXT)
DISTFILES += bigvec.c

CLEANFILES += bcast.o[0-9][0-9]*
CLEANFILES += bcast.e[0-9][0-9]*

DISTFILES += serial.gp
DISTFILES += ../serial-2.dat ../serial-4.dat ../serial-8.dat
DISTFILES += bcast.pbs
DISTFILES += bcast.gp

pbs: check-ALG check-P $(ALG)$(EXEEXT) bcast.pbs run-pbs

pbs-profile: check-ALG check-P $(ALG)$(EXEEXT) bcast-tau.pbs run-pbs-profile

run-pbs:
	$(MAKE) run-force-one ALG=$(ALG) P=$(P) SCRIPT=bcast.pbs

run-pbs-profile:
	$(MAKE) run-force-one ALG=$(ALG) P=$(P) SCRIPT=bcast-tau.pbs

run-force-one: check-script
	@if test x`whoami` = x"rvuduc3" ; then \
	  QUEUE=faculty ; \
	else \
	  QUEUE=class ; \
	fi ; \
	echo "===== [$(ALG), P=$(P)] Running '$(SCRIPT)' with QUEUE=$${QUEUE} =====" ; \
	if test x"$(SCRIPT)" = x"bcast-tau.pbs" ; then \
	  mkdir -p $(ALG)-$(P).tau-profile || exit 1 ; \
	  mkdir -p $(ALG)-$(P).tau-trace || exit 1 ; \
	fi ; \
	cat $(SCRIPT) \
	  | sed "s,-q class,-q $${QUEUE},g" \
	  | sed "s,bcast-2,$(ALG)-$(P),g" \
	  | sed "s,bcast,$(ALG)$(EXEEXT),g" \
	  | sed "s,-l nodes=2,-l nodes=$(P),g" \
	  | sed "s,-np 2 ,-np $(P) ,g" \
	  | $(DEBUG) qsub

check-script:
	@if test -z "$(SCRIPT)" ; then echo "*** Variable SCRIPT is not defined. ***" ; exit 1 ; fi
	@if ! test -f "$(SCRIPT)" ; then echo "*** $(SCRIPT) could not be found. ***" ; exit 1 ; fi

plot-alg: check-data-alg
	@if test -f "$(ALG).png" ; then echo "WARNING: Overwriting $(ALG).png ..." ; fi
	rm -f $(ALG).png
	cat serial.gp \
	    | sed 's,serial,$(ALG),g' \
	    | gnuplot
	@if test -f "$(ALG).png" ; then echo "==> Done! See: $(ALG).png" ; else exit 1 ; fi

plot-all: check-data-all
	@if test -f "bcast-$(P).png" ; then echo "WARNING: Overwriting bcast-$(P).png ..." ; fi
	rm -f bcast-$(P).png
	cat bcast.gp \
	    | sed 's,P=2,P=$(P),g' \
	    | sed 's,bcast-2,bcast-$(P),g' \
	    | sed 's,serial-2,serial-$(P),g' \
	    | sed 's,tree-2,tree-$(P),g' \
	    | sed 's,bigvec-2,bigvec-$(P),g' \
	    | gnuplot
	@if test -f "bcast-$(P).png" ; then echo "==> Done! See: bcast-$(P).png" ; else exit 1 ; fi

check-data-alg: check-ALG
	@if ! test -f $(ALG)-2.dat || ! test -f $(ALG)-4.dat || ! test -f $(ALG)-8.dat ; then \
	  echo "*** Missing data files to make a plot. Check that $(ALG)-{2,4,8}.dat exist and are valid. ***" ; \
	  exit 1 ; \
	fi

check-data-all: check-P
	@if ! test -f serial-$(P).dat || ! test -f tree-$(P).dat || ! test -f bigvec-$(P).dat ; then \
	  echo "*** Missing data files to make a plot. Check that {serial, tree, bigvec}-$(P).dat exist and are valid. ***" ; \
	  exit 1 ; \
	fi

check-ALG:
	@if test -z "$(ALG)" ; then \
	  echo "*** 'ALG' not specified. ***" ; \
	  $(MAKE) help ; \
	  exit 1 ; \
	fi
	@if ! test x"$(ALG)" = x"serial" \
	    && ! test x"$(ALG)" = x"tree" \
	    && ! test x"$(ALG)" = x"bigvec" \
	; then \
	  echo "*** 'ALG=$(ALG)' is not valid. ALG must be one of {serial, tree, bigvec}. ***" ; \
	  exit 1 ; \
	fi

check-P:
	@if test -z "$(P)" ; then \
	  echo "*** 'P' not specified. ***" ; \
	  $(MAKE) help ; \
	  exit 1 ; \
	fi
	@if ! test x"$(P)" = x"2" \
	    && ! test x"$(P)" = x"4" \
	    && ! test x"$(P)" = x"8" \
	; then \
	  echo "*** 'P=$(P)' is not valid. P must be one of {2, 4, 8}. ***" ; \
	  exit 1 ; \
	fi

dist: is-distroot-set distclean
	mkdir -p "$(DISTROOT)"
	cp $(DISTFILES) "$(DISTROOT)"/.
	tar cvf - "$(DISTROOT)"/* | gzip -9c > "$(DISTROOT).tgz"

distclean: is-distroot-set
	if test -d "$(DISTROOT)" ; then rm -rf "$(DISTROOT)" ; fi
	rm -f "$(DISTROOT).tgz"

is-distroot-set:
	@if test -z "$(DISTROOT)" ; then \
	  echo "*** Must specify DISTROOT, the base name of the output directory and tarball. ***" ; \
	  exit 1 ; \
	fi

clean:
	rm -f core *~ $(BINS) $(CLEANFILES)
	if test -n "$(DISTROOT)" ; then $(MAKE) distclean DISTROOT="$(DISTROOT)" ; fi

# eof
