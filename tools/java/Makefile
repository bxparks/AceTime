SRCS := TestDataGenerator.java BasicZones.java ExtendedZones.java
CLASSES := $(SRCS:.java=.class)

.SUFFIXES:.java .class

.java.class:
	javac $*.java

all: classes

classes: $(CLASSES)

run:
	java TestDataGenerator

BasicZones.java:
	../tzcompiler.sh --tag 2019a --action zonedb --language java --scope basic

ExtendedZones.java:
	../tzcompiler.sh --tag 2019a --action zonedb --language java --scope extended

clean:
	rm -f *.class
