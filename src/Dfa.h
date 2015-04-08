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

class Nfa;

using namespace std;

class Dfa {
public:
	Dfa(Nfa *);
	virtual ~Dfa();

	unordered_set<State *> getStates();
	unordered_set<State *> getFinals();
	State * getInitial();

	void minimize();

	int evaluate(string);
	void print();
	void toDot(char const *fileName);

	void minimise_hopcroft();

private:
	unordered_set<State *> states;
	unordered_set<State *> finals;
	State *initial;

	void determinize(Nfa *);
	void recDeterminize(Nfa *, map<set<State *>, State *> *, set<State *>);

	unordered_set<int> getSymbols();
	int recEvaluate(string, State *);

	void initializeStatesAfterMinimisation(list<unordered_set<State*> >);
};

#endif /* SRC_DFA_H_ */


/*
 *  I = une liste vide etats initiaux
 *	finaux.ajouter(initial);
 *  pour chaque etat
 *  	pour chaque c dqns symbol
 *  		etat_accesible = getTrabsition(c);
 *  		inverser transtion entre etat_accessible et etat
 * 		si etat est dans final
 * 			ajouter etat dans I
 *
 * 	si |I| > 1 alors
 * 		creer un etat bidon
 * 		ajouter les eps _trans vers tous etats dans I
 *
 *
 */
