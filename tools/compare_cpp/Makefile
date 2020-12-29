HINNANT_DATE_DIR := ../../../date

generate_data.out: generate_data.cpp
	$(CXX) -DHAS_REMOTE_API=1 -DAUTO_DOWNLOAD=0 \
		-I $(HINNANT_DATE_DIR)/include -Wall -std=gnu++11 -o $@ $^ \
		$(HINNANT_DATE_DIR)/src/*.cpp -lcurl

json: generate_data.out small.txt
	./generate_data.out \
		--tz_version 2019c \
		--start_year 2000 \
		--until_year 2038 \
		< small.txt

clean:
	rm -f generate_data.out validation_data.json
