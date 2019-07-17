tests:
	for i in */Makefile ; do make -C $$(dirname $$i) ; done

clean:
	for i in */Makefile ; do echo $$i ; make -C $$(dirname $$i) clean ; done

runtests:
	for i in */Makefile ; do $$(dirname $$i)/$$(dirname $$i).out ; done

validation:
	make -C BasicValidationUsingJavaTest/zonedb2018g/
	make -C ExtendedValidationUsingJavaTest/zonedbx2018g/
