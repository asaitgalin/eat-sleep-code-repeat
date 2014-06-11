CXX = clang++
CXXFLAGS = -Wall -std=c++0x
OUTPUT = wc

MAPREDUCE = ../../
TP_DIR = ../../../threadpool

all: $(OUTPUT)

$(OUTPUT): wordcount.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) -I$(MAPREDUCE) -I$(TP_DIR) -lboost_thread -lboost_system 
	
clean:
	$(RM) $(OUTPUT)

