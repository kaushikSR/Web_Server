
CFLAGS = -std=c++11 -Wall -O3   #-DHTTP_V11
LDFLAGS = -pthread 
CC = g++
SRC = WS.cpp HTTPParser.cpp
HDR = HTTPParser.h
OBJ = $(SRC:%.cpp=%.o)
#GDB = gdb --args

server: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp ${HDR}
	$(CC) $(CFLAGS) -c $<

SERVER = 10.0.0.1
PORT = 9000
OUT = output
DIR = ${PWD}
TIMEOUT = 10

RATE = 5000

.PHONY: run
run: 
	$(GDB) ./server -dir ${DIR}/html -port ${PORT}


.PHONY: clean
clean:
	rm -rf *.o

.PHONY: clean_all cleanall
cleanall clean_all:
	$(MAKE) -C . clean
	rm -rf server
