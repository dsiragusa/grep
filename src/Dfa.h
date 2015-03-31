/*
 * Dfa.h
 *
 *  Created on: 31/mar/2015
 *      Author: daniele
 */

#ifndef SRC_DFA_H_
#define SRC_DFA_H_

#include <unordered_set>
#include "State.h"
#include "Nfa.h"

using namespace std;

class Dfa {
public:
	Dfa(Nfa *);
	virtual ~Dfa();


private:
	unordered_set<State *> states;
	State *initial, *final;

	void determinize(Nfa *);
	void rec_determinize(Nfa *, map<set<State *>, State *>, set<State *>);
};

#endif /* SRC_DFA_H_ */
