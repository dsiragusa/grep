/*
 * Dfa.cpp
 *
 *  Created on: 31/mar/2015
 *      Author: daniele
 */

#include "Dfa.h"

Dfa::Dfa(Nfa *toDeterminize) {
	determinize(toDeterminize);
}

Dfa::~Dfa() {
	// TODO Auto-generated destructor stub
}

void Dfa::determinize(Nfa *toDeterminize) {
	map<set<State *>, State *> *superSet = new map<set<State *>, State *>();

	set<State *> initialId;
	initialId.insert(toDeterminize->getInitial());

	rec_determinize(toDeterminize, superSet, initialId);
	initial = superSet->find(initialId)->second;
}

void Dfa::rec_determinize(Nfa *toDeterminize, map<set<State *>, State *> *superSet, set<State *> currentId) {
	if (superSet->find(currentId) != superSet->end())
		return;

	State *current = new State();
	superSet->emplace(currentId, current);
	states.insert(current);

	map<int, set<State *> > transitions;

	for (auto state : currentId) {
		//check if final and add to finals!

		for (auto symbol : state->get_symbols()) {
			if (transitions.find(symbol) == transitions.end())
				transitions.emplace(symbol, state->getTransitions(symbol));
			else {
				auto nextStates = state->getTransitions(symbol);
				transitions.find(symbol)->second.insert(nextStates.begin(), nextStates.end());
			}
		}
	}

	for (auto transition : transitions) {
		rec_determinize(toDeterminize, superSet, transition.second);
		current->setTransition(transition.first, superSet->find(transition.second)->second);
	}
}

int main () {

}
