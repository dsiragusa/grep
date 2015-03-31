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
	map<set<State *>, State *> superSet;
	initial = new State();

	set<State *> initialId;
	initialId.insert(toDeterminize->getInitial());
	superSet.emplace(initialId, superSet);

}

void Dfa::rec_determinize(Nfa *toDeterminize, map<set<State *>, State *> superSet, set<State *> currentId) {
	if (superSet.find(currentId) == superSet.end())
		superSet.emplace(currentId, new State());

	State *current = superSet.find(currentId)->second;

	for (auto state : toDeterminize->g)
	for (auto symbol : toDeterminize->current->get_symbols()) {
		set<int> nextStatesId;
		for (auto nextState : current->getTransitions(symbol))
			nextStatesId.insert(nextState->getId());

		current
	}
}

int main () {

}
