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

enum card_t {KLEENE_STAR, OPTION, PLUS};

class Nfa {
	private:
	Nfa(const Nfa *);
	//map<int, map<char,list<int> > > transitions;
	unordered_set<State *> states;
	State *final;
	State *initial;

	int rec_evaluate(string, State *);

	public:
	Nfa(char);
	virtual ~Nfa();
	void concatenate(const Nfa *);
	void unify(const Nfa *);
	void apply_cardinality(enum card_t);
	void apply_cardinality(int, int);

	void print();
	int evaluate(string);
};

#endif /* SRC_NFA_H_ */
