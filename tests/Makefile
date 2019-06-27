tests:
	for i in */Makefile ; do make -C $$(dirname $$i) -j 4 ; done

clean:
	for i in */Makefile ; do echo $$i ; make -C $$(dirname $$i) clean ; done

runtests:
	for i in */Makefile ; do $$(dirname $$i)/$$(dirname $$i).out ; done

validation_data:
	make -C BasicValidationUsingJavaTest/zonedb2018g/
	make -C ExtendedValidationUsingJavaTest/zonedbx2018g/
	make -C BasicValidationUsingJavaTest validation_data
	make -C BasicValidationUsingPythonTest validation_data
	make -C ExtendedValidationUsingJavaTest validation_data
	make -C ExtendedValidationUsingPythonTest validation_data
