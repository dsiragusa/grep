/*
 * State.h
 *
 *  Created on: 28/mar/2015
 *      Author: Daniele
 */

#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <iostream>
#include <cstdio>
#include <map>
#include <list>
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
	void delete_transition(int,State*);
	unordered_set<State *> getTransitions(int);
	void setTransition(int, State *);
	void copyTransitions(const State *);
	void adaptTransitions(const State *, const map<State *, State *> *);
	void print();
	void toDot(FILE *);
	list<int> get_symbols();
private:
	static UniqueIdGenerator idGen;

	int id;
	map<int, unordered_set<State *> > transitions;
	string getChar(int);
};

#endif /* SRC_STATE_H_ */
