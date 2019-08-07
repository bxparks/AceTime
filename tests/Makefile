tests:
	for i in *Test/Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		make -C $$(dirname $$i) -j; \
	done

runtests:
	for i in *Test/Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$$(dirname $$i)/$$(dirname $$i).out; \
	done

clean:
	for i in *Test/Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		make -C $$(dirname $$i) clean; \
	done
