/*
 * Dfa.h
 *
 *  Created on: 31/mar/2015
 *      Author: Daniele
 */

#ifndef SRC_DFA_H_
#define SRC_DFA_H_

#include <set>
#include <list>
#include <unordered_set>
#include "State.h"
#include "Nfa.h"

using namespace std;

class Dfa {
public:
	Dfa(Nfa *);
	virtual ~Dfa();
	void print();
	void minimise_hopcroft();
	int evaluate(string);
	int rec_evaluate(string, State *);
	void toDot(char const *fileName);
private:
	unordered_set<State *> states;
	State *initial;
	unordered_set<State *> finals;
	bool isMinimal=false;

	void determinize(Nfa *);
	void rec_determinize(Nfa *, map<set<State *>, State *> *, set<State *>);
	void initializeStatesAfterMinimisation(list<unordered_set<State*> >);
	unordered_set<State *> unreachable_states();
	unordered_set<int> getSymbols();
};

#endif /* SRC_DFA_H_ */
