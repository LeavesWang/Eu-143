CFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)

BIN=Root2Mca CofFromMca Root2Ana Root2AnaP Root2AnaS

default: $(BIN)

$(BIN): % : %.cpp 
	g++ -o $@ $< -g -Wall -O3 -std=c++11 -lMinuit $(CFLAGS) $(ROOTLIBS)

clean:
	rm $(BIN)