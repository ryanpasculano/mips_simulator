TESTS = lsq_test  rs_test freelist_test


CPPFLAGS= -std=c++11 -g -O2


%.o: %.cpp Makefile
	$(CXX) $(CPPFLAGS) -c  -I include/ $< -o $@

all: $(TESTS) 

lsq_test: lsq_test.cpp ./../src/lsq.o
	$(CXX) $(CPPFLAGS) $^ -o $@


freelist_test: freelist_test.cpp ./../src/freelist.o
	$(CXX) $(CPPFLAGS) $^ -o $@

rs_test: rs_test.cpp ./../src/rs.o
	$(CXX) $(CPPFLAGS) $^ -o $@



$(SIMULATOR): $(SIMULATOR_OBJS)
	$(CXX) $(CPPFLAGS)  $^ -o $@ libjsoncpp.a -I include/

clean:
	rm -rf *~ src/*.o $(TESTS) *.out 
