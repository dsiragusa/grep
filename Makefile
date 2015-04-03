CC = g++
C11FLAG = -std=c++0x -Wno-deprecated-register

all: grep

dfa: obj/State.o obj/UniqueIdGenerator.o obj/Nfa.o obj/Dfa.o
	$(CC) $^ -o bin/$@ 

nfa: obj/State.o obj/UniqueIdGenerator.o obj/Nfa.o
	$(CC) $^ -o bin/$@ 

obj/%.o: src/%.cpp
	$(CC) -c $< -o $@ $(C11FLAG)

grep: src/lex.l src/parser.y obj/State.o obj/UniqueIdGenerator.o obj/Nfa.o
	flex src/lex.l
	bison -d -r all src/parser.y
	$(CC) lex.yy.c parser.tab.c obj/State.o obj/UniqueIdGenerator.o obj/Nfa.o -o $@ $(C11FLAG) -ll

clean:
	rm -rf bin/* obj/*
