CXX = clang++
CXXFLAGS = -Wall -std=c++0x
OUTPUT = pmi

MAPREDUCE = ../../
TP_DIR = ../../../threadpool

all: $(OUTPUT)

$(OUTPUT): pmi.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) -I$(MAPREDUCE) -I$(TP_DIR) -lboost_thread -lboost_system 
	
clean:
	$(RM) $(OUTPUT)

