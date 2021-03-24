SRCS := GenerateData.java
CLASSES := $(SRCS:.java=.class)

.SUFFIXES:.java .class

.java.class:
	javac $*.java

all: classes

classes: $(CLASSES)

validation_data.json: $(CLASSES) small.txt
	java -cp . GenerateData \
		--start_year 2000 \
		--until_year 2038 \
		< small.txt \
		> validation_data.json

clean:
	rm -f *.class validation_data.*
