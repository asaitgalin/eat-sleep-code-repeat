CXX = g++
CXXFLAGS = -std=c++0x -O3 -Wall
RM = rm -f
OUT = heaps_test

all: $(OUT)

$(OUT): tests.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) 

clean:
	$(RM) $(OUT)

