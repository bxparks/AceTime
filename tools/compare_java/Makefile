SRCS := TestDataGenerator.java
CLASSES := $(SRCS:.java=.class)

.SUFFIXES:.java .class

.java.class:
	javac $*.java

all: classes

classes: $(CLASSES)

json: $(CLASSES)
	java -cp . TestDataGenerator \
		--scope basic \
		--db_namespace zonedb \
		--start_year 2000 \
		--until_year 2038 \
		--format json \
		< small.txt

clean:
	rm -f *.class validation_data.json
