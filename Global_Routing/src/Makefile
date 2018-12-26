CPP = g++
CPPFLAGS = -Wall -std=c++11 -O3
HERDERS = GLOBALROUTE.h NET.h EDGE.h BIN.h
SOURCES = main.cpp GLOBALROUTE.cpp NET.cpp EDGE.cpp BIN.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = ../bin/route

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(OBJECTS) -o $@

%.o: %.cpp 
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean: 
	rm -rf *.o $(EXECUTABLE)