tests:
	set -e; \
	for i in *Test/Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) -j; \
	done

runtests:
	set -e; \
	for i in *Test/Makefile; do \
		if [ "$$i" != "BasicZoneProcessorTest/Makefile" -a \
			"$$i" != "ExtendedZoneProcessorMoreTest/Makefile" -a \
			"$$i" != "ExtendedZoneProcessorTest/Makefile" \
		]; then \
			echo '==== Running:' $$(dirname $$i); \
			$$(dirname $$i)/$$(dirname $$i).out; \
		fi; \
	done

clean:
	set -e; \
	for i in *Test/Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) clean; \
	done
