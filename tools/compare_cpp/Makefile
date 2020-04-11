HINNANT_DATE_DIR := ../../../date

test_data_generator.out: test_data_generator.cpp
	$(CXX) -DHAS_REMOTE_API=1 -DAUTO_DOWNLOAD=0 \
		-I $(HINNANT_DATE_DIR)/include -Wall -std=gnu++11 -o $@ $^ \
		$(HINNANT_DATE_DIR)/src/*.cpp -lcurl
