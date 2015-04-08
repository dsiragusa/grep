/*
 * Nfa.h
 *
 *  Created on: 28/mar/2015
 *      Author: Daniele
 */

#ifndef SRC_NFA_H_
#define SRC_NFA_H_

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <set>
#include <forward_list>
#include "Dfa.h"
#include "State.h"

using namespace std;
#define ACCEPT 1
#define REJECT 0

#define UNBOUNDED -1
#define SIMPLE_COUNT -2

enum card_t {KLEENE_STAR, OPTION, PLUS};

class Nfa {
	private:
	unordered_set<State *> states;
	State *initial;
	unordered_set<State*> finals;

	int recEvaluate(string, State *);

	public:
	Nfa(int);
	Nfa(const Nfa *);
	Nfa(forward_list<int>, bool);
	Nfa(Dfa *);
	virtual ~Nfa();

	void concatenate(const Nfa *);
	void unify(const Nfa *);
	void applyCardinality(enum card_t);
	void applyCardinality(int, int);

	unordered_set<State *> getStates();
	unordered_set<State*> getFinals();
	State * getInitial();
	State * getFinal();

	void eliminateEps();
	State * getStateWithEpsTransitions();
	bool isAccessible(State*);

	void print();
	void toDot(char const *);
	int evaluate(string);
};

#endif /* SRC_NFA_H_ */
