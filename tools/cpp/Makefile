HINNANT_DATE_DIR := ../../../date

test_data_generator.out: test_data_generator.cpp
	$(CXX) -I $(HINNANT_DATE_DIR)/include -Wall -std=gnu++11 -o $@ $^ \
		$(HINNANT_DATE_DIR)/src/*.cpp -lcurl
