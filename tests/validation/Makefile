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

# Build the validation tests in series. This was the original 'tests' target
# but as the validation tests grow slower and slower (mostly due to the new
# implementation of compare_pytz and compare_dateutil), I wrote the paralel
# version above to utilize multiple CPUs. I thought the serial version would be
# useful for GitHub Actions, but even there it seems like the workflow runners
# are allocated least 2 CPUs, and using the parallel version above reduces the
# execution time from 11 min (serial) to 6 min (parallel). So I ended up just
# using the parallel version even for Actions workflow. This target is retained
# for historical reference.
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
