/*
 * Nfa.h
 *
 *  Created on: 28/mar/2015
 *      Author: Daniele
 */

#ifndef SRC_NFA_H_
#define SRC_NFA_H_

#include <iostream>
#include <unordered_set>
#include <set>
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
	State *initial, *final;
	unordered_set<State*> finals;

	int rec_evaluate(string, State *);
	int rec_evaluate_second(string,State*);
	void print_finals();

	public:
	Nfa(int);
	Nfa(list<int>, bool);
	virtual ~Nfa();

	void startAnywhere();
	void endAnywhere();
	void concatenate(const Nfa *);
	void unify(const Nfa *);
	void apply_cardinality(enum card_t);
	void apply_cardinality(int, int);

	unordered_set<State *> getStates();
	State * getInitial();
	State * getFinal();

	void eliminate_eps();
	list<State*> getStatesWithEpSTransition();

	void print();
	int evaluate(string);
	Nfa(const Nfa *);
	bool isAccessible(State*);
	unordered_set<State*> getFinals();
};

#endif /* SRC_NFA_H_ */
