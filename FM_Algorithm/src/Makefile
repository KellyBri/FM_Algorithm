CPP = g++ -std=c++11
CPPFLAGS = -Wall
HERDERS = CELL.h NET.h
SOURCES = main.cpp CELL.cpp NET.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(OBJECTS) -o $@ -O3

%.o: %.cpp $(HERDERS)
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean: 
	rm -rf *.o $(EXECUTABLE)