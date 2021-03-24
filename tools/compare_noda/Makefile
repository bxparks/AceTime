TZ_VERSION = 2021a
START_YEAR := 2000
UNTIL_YEAR := 2050

# Location of the https://github.com/nodatime/nodatime repo.
NODA_TIME_DIR := $(HOME)/dev/nodatime

validation_data.json: Program.cs small.txt tzdata$(TZ_VERSION).nzd
	dotnet run -- \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		--nzd_file tzdata$(TZ_VERSION).nzd \
		< small.txt \
		> validation_data.json

# Generate the custom tzdata{xxx}.nzd file, depending on TZ_VERSION.
tzdata$(TZ_VERSION).nzd:
	dotnet run \
		-p $(NODA_TIME_DIR)/src/NodaTime.TzdbCompiler/NodaTime.TzdbCompiler.csproj \
		-- \
		-s https://data.iana.org/time-zones/releases/tzdata$(TZ_VERSION).tar.gz \
		-w $(NODA_TIME_DIR)/data/cldr/windowsZones-38-1.xml \
		-o $@

clean:
	rm -f validation_data.json tzdata$(TZ_VERSION).nzd
