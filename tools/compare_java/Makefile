SRCS := TestDataGenerator.java
CLASSES := $(SRCS:.java=.class)

.SUFFIXES:.java .class

.java.class:
	javac $*.java

all: classes

classes: $(CLASSES)

data: $(CLASSES) small.txt
	java -cp . TestDataGenerator \
		--scope basic \
		--db_namespace zonedb \
		--start_year 2000 \
		--until_year 2038 \
		--format cpp \
		< small.txt

json: $(CLASSES) small.txt
	java -cp . TestDataGenerator \
		--scope basic \
		--db_namespace zonedb \
		--start_year 2000 \
		--until_year 2038 \
		--format json \
		< small.txt

clean:
	rm -f *.class validation_data.* validation_tests.*
