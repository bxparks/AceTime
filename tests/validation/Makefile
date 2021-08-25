SHELL=bash

# There are 2 sets of validation test targets in this Makefile.
#
# 1) The minimal validation tests: BasicHinnantDateTest and
# ExtendedHinnantDateTest. These are the only 2 validation tests which can be
# configured to validate against a specific TZ version. These are used in the
# GitHub CI workflow.
#
# $ make validations
# $ make runvalidations
#
# 2) The full validation tests (6 x Basic*Test and 6 Extended*Test). Some of
# these will be broken when a new TZ DB is released because the dependent
# library has not released a new version of the library yet.
#
# $ make tests (or make tests-serial)
# $ make runtests
#
# Sometimes `make tests` fails because it tries to build the targets in
# parallel. In that case, we can use `make tests-serial`.

# Build just BasicHinnantDateTest and ExtendedHinnantDateTest tests in series.
validations:
	(set -e; \
	trap 'kill 0' SIGHUP SIGINT SIGQUIT SIGKILL SIGTERM; \
	for i in *HinnantDateTest/Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i); \
	done; \
	wait)

# Run the BasicHinnantDateTest and ExtendedHinnantDateTest validation tests.
# These are used in the GitHub CI workflow.
runvalidations:
	set -e; \
	for i in *HinnantDateTest/Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$$(dirname $$i)/$$(dirname $$i).out; \
	done

# Build *all* validation tests in parallel for reduced waiting time.
tests:
	(set -e; \
	trap 'kill 0' SIGHUP SIGINT SIGQUIT SIGKILL SIGTERM; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) & \
	done; \
	wait)

# Same as 'make tests' but in series not in parallel. I thought the serial
# version would be useful for GitHub Actions, but even there it seems like the
# workflow runners are allocated least 2 CPUs, and using the parallel version
# above reduces the execution time from 11 min (serial) to 6 min (parallel).
# Sometimes the serial version is needed when `make tests` failes due a race
# condition.
tests-serial:
	set -e; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) ; \
	done;

# Run *all* validation tests.
runtests:
	set -e; \
	for i in */Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$$(dirname $$i)/$$(dirname $$i).out; \
	done

# Clean all validation tests.
clean:
	set -e; \
	for i in */Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) clean; \
	done
