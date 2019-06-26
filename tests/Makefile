tests:
	for i in */Makefile ; do make -C $$(dirname $$i) -j 4 ; done

clean:
	for i in */Makefile ; do echo $$i ; make -C $$(dirname $$i) clean ; done

runtests:
	for i in */Makefile ; do $$(dirname $$i)/$$(dirname $$i).out ; done
