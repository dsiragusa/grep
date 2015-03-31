/*
 * Nfa.h
 *
 *  Created on: 28/mar/2015
 *      Author: daniele
 */

#ifndef SRC_NFA_H_
#define SRC_NFA_H_

#include <iostream>
#include <map>
#include <unordered_set>
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
	State *final;
	State *initial;
	unordered_set<State*> finals;

	int rec_evaluate(string, State *);

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
	void eliminate_eps();
	list<State*> getStatesWithEpSTransition();
	Nfa* determinize();
	void print();
	int evaluate(string);
	int rec_evaluate_second(string);
	Nfa(const Nfa *);
	bool isAccessible(State*);
};

#endif /* SRC_NFA_H_ */
