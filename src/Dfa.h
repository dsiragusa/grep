/*
 * Dfa.h
 *
 *  Created on: 31/mar/2015
 *      Author: daniele
 */

#ifndef SRC_DFA_H_
#define SRC_DFA_H_

#include <set>
#include "State.h"
#include "Nfa.h"

using namespace std;

class Dfa {
public:
	Dfa(Nfa *);
	virtual ~Dfa();
	void print();


private:
	unordered_set<State *> states;
	State *initial;
	unordered_set<State *> finals;

	void determinize(Nfa *);
	void rec_determinize(Nfa *, map<set<State *>, State *> *, set<State *>);
	void print_finals();
};

#endif /* SRC_DFA_H_ */
