/*
 * Nfa.cpp
 *
 *  Created on: 28/mar/2015
 *      Author: daniele
 */

#include "Nfa.h"

Nfa::Nfa(char symbol) {
	initial = new State();
	final = new State();
	initial->setTransition(symbol, final);
	states.insert(initial);
	states.insert(final);
}

Nfa::Nfa(const Nfa *toCopy) {
	map<State *, State *> oldToNewStates;
	for (auto& state : toCopy->states) {
		oldToNewStates.emplace(state, new State());
	}

	auto stateToCopy = toCopy->states.begin();
	auto myState = states.begin();

	while (stateToCopy != toCopy->states.end()) {
		(*myState)->adaptTransitions(*stateToCopy, &oldToNewStates);

		stateToCopy++;
		myState++;
	}
	initial = oldToNewStates.find(toCopy->initial)->second;
	final = oldToNewStates.find(toCopy->final)->second;
}

void Nfa::concatenate(const Nfa *toAppend) {
	states.insert(toAppend->states.begin(), toAppend->states.end());
	states.erase(toAppend->initial);
	(*states.find(final))->copyTransitions(toAppend->initial);

	final = toAppend->final;
}

void Nfa::unify(const Nfa *toUnify) {
	State *newInitial = new State();
	State *newFinal = new State();
	states.insert(toUnify->states.begin(), toUnify->states.end());
	states.insert(newInitial);
	states.insert(newFinal);

	newInitial->setEpsTransition(initial);
	newInitial->setEpsTransition(toUnify->initial);

	final->setEpsTransition(newFinal);
	toUnify->final->setEpsTransition(newFinal);

	initial = newInitial;
	final = newFinal;
}

void Nfa::apply_cardinality(enum card_t type) {
	State *newInitial = new State();
	State *newFinal = new State();
	states.insert(newInitial);
	states.insert(newFinal);

	//create e-transitions to initial and new_final
	//list<State *> eps_edges;

	(*states.find(final))->setEpsTransition(newFinal);
	switch (type) {
		case PLUS:	//final -> newFinal, initial | newInitial -> initial
		case KLEENE_STAR: //final -> newFinal, initial | newInitial -> initial, newFinal
			final->setEpsTransition(initial);
			break;
		case OPTION:	//final -> newFinal
			//add it to final
			eps_edges.push_back(initial);
			break;
	}

	//add them to the new initial state
	map<char, list<int> > new_init_trans;
	if (type == PLUS) {
		eps_edges.remove(new_final);
	}
	new_init_trans.emplace(EPS, eps_edges);

	//add new states to nfa
	transitions.emplace(new_initial, new_init_trans);
	transitions.emplace(new_final, map<char, list<int> >());

	initial = new_initial;
	final = new_final;
}

int Nfa::rec_evaluate(string in, State *state) {
	if (in.length() == 0) {
		cout << "empty string: state " << state->getId() << "\n";
		if (state == final)
			return ACCEPT;

		list<State *> eps_states = state->getEpsTransitions();
		for (auto& eps_state : eps_states)
			if (rec_evaluate(in, eps_state) == ACCEPT)
				return ACCEPT;

		return REJECT;
	}

	list<State *> next_states = state->getTransitions(in.at(0));
	list<State *> eps_states = state->getEpsTransitions();

	for (auto& next_state : next_states)
		if (rec_evaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;

	for (auto& eps_state : eps_states)
		if (rec_evaluate(in, eps_state) == ACCEPT)
			return ACCEPT;

	return REJECT;
}

int Nfa::evaluate(string in) {
	int result = rec_evaluate(in, initial);
	cout << "\n" << in << ": " << ((result == ACCEPT) ? "YES" : "NO") << "\n\n";
	return result;
}

Nfa::~Nfa() {
	// TODO Auto-generated destructor stub
}

void Nfa::print() {
	cout << "Initial: " << initial->getId() << ", final: " << final->getId() << "\n";
	for (auto& state : states) {
		state->print();
	}
	cout << "\n\n";
}


int main () {
	Nfa *a = new Nfa('a');
	a->print();
	a->evaluate("a");

	Nfa *b = new Nfa('b');
	b->print();
	b->evaluate("b");
	b->evaluate("a");

	a->unify(b);
	//a->concatenate(b);
	a->print();
	a->evaluate("a");
	a->evaluate("b");
	a->evaluate("ab");

}

