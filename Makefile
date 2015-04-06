CC = g++
C11FLAG = -g -std=c++0x -Wno-deprecated-register

all: grep

grep: src/lex.l src/parser.y obj/State.o obj/UniqueIdGenerator.o obj/Nfa.o obj/Dfa.o obj/Tree.o obj/BracketExprManager.o
	flex src/lex.l
	bison -d -r all src/parser.y
	$(CC) lex.yy.c parser.tab.c obj/State.o obj/UniqueIdGenerator.o obj/Dfa.o obj/Nfa.o obj/Tree.o obj/BracketExprManager.o -o $@ $(C11FLAG) -ll

obj/%.o: src/%.cpp
	$(CC) -c $< -o $@ $(C11FLAG)

clean:
	rm -rf bin/* obj/*
