SIMULATOR = simulator
TESTS = #lsq_test freelist_test
SIMULATOR_SOURCES := $(wildcard src/*.cpp)
SIMULATOR_OBJS := $(SIMULATOR_SOURCES:%.cpp=%.o)

JSON_LIB=libjsoncpp.a

CPPFLAGS= -std=c++11 -g -O0


%.o: %.cpp Makefile
	$(CXX) $(CPPFLAGS) -c  -I include/ $< -o $@

all: $(TESTS) $(SIMULATOR) 

#lsq_test: tests/lsq_test.cpp src/lsq.o
#	$(CXX) $(CPPFLAGS) $^ -o $@

#freelist_test: tests/freelist_test.cpp src/freelist.o
#	$(CXX) $(CPPFLAGS) $^ -o $@



$(SIMULATOR): $(SIMULATOR_OBJS)
	$(CXX) $(CPPFLAGS)  $^ -o $@ libjsoncpp.a -I include/

clean:
	rm -rf *~ src/*.o $(SIMULATOR) $(TESTS) *.out 
