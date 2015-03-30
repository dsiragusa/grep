CC = g++
C11FLAG = -std=c++0x

all: nfa

nfa: obj/State.o obj/UniqueIdGenerator.o obj/Nfa.o
	$(CC) $^ -o bin/$@ 

obj/%.o: src/%.cpp
	$(CC) -c $< -o $@ $(C11FLAG)
	
clean:
	rm -rf bin/* obj/*
