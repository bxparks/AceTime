SRCS := TestDataGenerator.java
CLASSES := $(SRCS:.java=.class)

.SUFFIXES:.java .class

.java.class:
	javac $*.java

all: classes

classes: $(CLASSES)

json: $(CLASSES) small.txt
	java -cp . TestDataGenerator \
		--start_year 2000 \
		--until_year 2038 \
		< small.txt

clean:
	rm -f *.class validation_data.* validation_tests.*
