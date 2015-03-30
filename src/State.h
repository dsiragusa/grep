/*
 * State.h
 *
 *  Created on: 28/mar/2015
 *      Author: daniele
 */

#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <iostream>
#include <map>
#include <list>
#include <exception>
#include "UniqueIdGenerator.h"

using namespace std;

class State {
public:
	State();
	virtual ~State();

	int getId();

	list<State *> getTransitions(char);
	list<State *> getEpsTransitions();
	void setTransition(char, State *);
	void setEpsTransition(State *);
	void copyTransitions(const State *);
	void adaptTransitions(const State *, const map<State *, State *> *);
	void print();

private:
	static const char EPS;
	static UniqueIdGenerator idGen;

	int id;
	map<char, list<State *> > transitions;
};

#endif /* SRC_STATE_H_ */
