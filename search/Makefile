CXX = g++
CXXFLAGS = -Wall
RM = rm -f
OUT = search

STEMMER_HEADER = stemmer/libstemmer.h
STEMMER_LIB = stemmer/libstemmer.o

all: $(OUT)

$(OUT): search_utils.cpp search_engine.cpp main.cpp
	$(CXX) $^ $(STEMMER_LIB) $(CXXFLAGS) -o $@

clean:
	$(RM) $(OUT)

