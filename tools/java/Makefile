TARGETS := BasicZones.java ExtendedZones.java

all: $(TARGETS)
	javac *.java

run:
	java TestDataGenerator

BasicZones.java:
	../tzcompiler.sh --tag 2019a --action zonedb --language java --scope basic

ExtendedZones.java:
	../tzcompiler.sh --tag 2019a --action zonedb --language java --scope extended
