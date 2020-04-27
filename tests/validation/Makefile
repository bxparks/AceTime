SHELL=/bin/bash

# Build the validation tests in parallel for reduced waiting time.
tests:
	(set -e; \
	trap 'kill 0' SIGHUP SIGINT SIGQUIT SIGKILL SIGTERM; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		make -C $$(dirname $$i) & \
	done; \
	wait)

# Build the validation tests in series, for continuous integration pipelines.
tests-serial:
	set -e; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		make -C $$(dirname $$i) ; \
	done;

runtests:
	set -e; \
	for i in */Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$$(dirname $$i)/$$(dirname $$i).out; \
	done

clean:
	set -e; \
	for i in */Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		make -C $$(dirname $$i) clean; \
	done
