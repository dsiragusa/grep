#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <iostream>
#include <cstdio>
#include <map>
#include <forward_list>
#include <unordered_set>
#include <exception>
#include "UniqueIdGenerator.h"

using namespace std;

class State {
public:
	static const int EPS;
	static const int DOT;

	State();
	virtual ~State();
	int getId();
	void deleteTransition(int,State*);
	unordered_set<State *> getTransitions(int);
	void setTransition(int, State *);
	void copyTransitions(const State *);
	void adaptTransitions(const State *, const map<State *, State *> *);
	void print();
	void toDot(FILE *, char const *);
	forward_list<int> getSymbols();
	map<int, unordered_set<State *> > getAllTransitions();
private:
	static UniqueIdGenerator idGen;

	int id;
	map<int, unordered_set<State *> > transitions;
	string getChar(int);
};

#endif /* SRC_STATE_H_ */
