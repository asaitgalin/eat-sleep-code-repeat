CXX = g++
CXXFLAGS = -std=c++0x -O3
RM = rm -f

OUTEXEC = tests

all: $(OUTEXEC) 

$(OUTEXEC): tests.cpp pool.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ -pthread -lboost_unit_test_framework

clean:
	$(RM) $(OUTEXEC)

