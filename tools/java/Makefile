SRCS := TestDataGenerator.java
CLASSES := $(SRCS:.java=.class)

.SUFFIXES:.java .class

.java.class:
	javac $*.java

all: classes

classes: $(CLASSES)

run:
	java TestDataGenerator

clean:
	rm -f *.class
