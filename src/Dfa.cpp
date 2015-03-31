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
		if (toDeterminize->getFinals().find(state) != toDeterminize->getFinals().end())
			finals.insert(current);

		for (auto symbol : state->get_symbols()) {
			auto nextStates = state->getTransitions(symbol);
			if (transitions.find(symbol) == transitions.end()) {
				auto tmp = set<State *> (nextStates.begin(), nextStates.end());
				transitions.emplace(symbol, tmp);
			}
			else {
				transitions.find(symbol)->second.insert(nextStates.begin(), nextStates.end());
			}
		}
	}

	for (auto transition : transitions) {
		rec_determinize(toDeterminize, superSet, transition.second);
		current->setTransition(transition.first, superSet->find(transition.second)->second);
	}
}

void Dfa::print_finals() {
	cout<<", final:  ";
		for(auto& f:finals){
			cout<< f->getId()<< " , ";
		}
		cout<<"\n";
}

void Dfa::print() {
	cout << "Initial: " << initial->getId();
	print_finals();
	for (auto& state : states) {
		state->print();
	}
	cout << "\n\n";
}

int main () {
	Nfa *a = new Nfa('a');
	Nfa *b = new Nfa('b');
	Nfa *c = new Nfa('c');

	a->concatenate(b);
	a->concatenate(c);
	a->startAnywhere();
	a->endAnywhere();

	a->print();

	a->eliminate_eps();

	a->print();

	Dfa *dfa = new Dfa(a);
	dfa->print();

}
