/*
 * Nfa.cpp
 *
 *  Created on: 28/mar/2015
 *      Author: Daniele
 */

#include "Nfa.h"

Nfa::Nfa(int symbol) {
	initial = new State();
	states.insert(initial);

	State *final = new State();
	states.insert(final);
	finals.insert(final);

	initial->setTransition(symbol, final);
}

Nfa::Nfa(forward_list<int> symbols, bool areMatching) {
	initial = new State();
	states.insert(initial);

	State *final = new State();
	states.insert(final);
	finals.insert(final);

	if ( ! areMatching) {
		unordered_set<int> toRefuse(symbols.begin(), symbols.end());
		symbols.clear();
		for (int i = 0; i < 127; i++) {
			if (toRefuse.find(i) == toRefuse.end())
				symbols.push_front(i);
		}
	}

	for (auto& symbol : symbols) {
		initial->setTransition(symbol, final);
	}
}

Nfa::Nfa(const Nfa *toCopy) {
	map<State *, State *> oldToNewStates;

	for (auto& state : toCopy->states) {
		State *newState = new State();
		states.insert(newState);
		oldToNewStates.emplace(state, newState);
	}

	auto stateToCopy = toCopy->states.begin();

	while (stateToCopy != toCopy->states.end()) {
		auto newState = oldToNewStates[*stateToCopy];
		newState->adaptTransitions(*stateToCopy, &oldToNewStates);

		stateToCopy++;
	}

	initial = oldToNewStates[toCopy->initial];
	for (auto final : toCopy->finals)
		finals.insert(oldToNewStates[final]);
}

Nfa::Nfa(Dfa *toTranspose) {
	map<State *, State *> mapStates;

	for (auto dfaState : toTranspose->getStates()) {
		State *newState = new State();
		states.insert(newState);
		mapStates.emplace(dfaState, newState);
	}

	finals.insert(mapStates[toTranspose->getInitial()]);

	for (auto dfaState : toTranspose->getStates()) {
		for (auto trans : dfaState->getAllTransitions()) {
			State *dest = *(trans.second.begin());
			State *newDest = mapStates[dest];
			newDest->setTransition(trans.first, mapStates[dfaState]);
		}
	}

	if (toTranspose->getFinals().size() > 1) {
		initial = new State();
		states.insert(initial);
		for (auto dfaFinal : toTranspose->getFinals())
			initial->setTransition(State::EPS, mapStates[dfaFinal]);

		eliminateEps();
	}
	else {
		State *dfaFinal = *(toTranspose->getFinals().begin());
		initial = mapStates[dfaFinal];
	}
}

Nfa::~Nfa() {
	for (auto state : states)
		delete state;
}

void Nfa::concatenate(const Nfa *toAppend) {
	states.insert(toAppend->states.begin(), toAppend->states.end());
	states.erase(toAppend->initial);

	State *final = *finals.begin();
	final->copyTransitions(toAppend->initial);

	delete toAppend->initial;

	finals.clear();
	finals.insert(toAppend->finals.begin(), toAppend->finals.end());
}

void Nfa::unify(const Nfa *toUnify) {
	State *newInitial = new State();
	State *newFinal = new State();
	states.insert(toUnify->states.begin(), toUnify->states.end());
	states.insert(newInitial);
	states.insert(newFinal);

	newInitial->setTransition(State::EPS, initial);
	newInitial->setTransition(State::EPS, toUnify->initial);

	State *final1 = *finals.begin();
	State *final2 = *toUnify->finals.begin();
	final1->setTransition(State::EPS, newFinal);
	final2->setTransition(State::EPS, newFinal);

	initial = newInitial;
	finals.clear();
	finals.insert(newFinal);
}

void Nfa::eliminateEps() {
	State * epsTransState;

	while ((epsTransState = getStateWithEpsTransitions()) != NULL) {
		for (auto epsDest : epsTransState->getTransitions(State::EPS)) {
			if (finals.find(epsDest) != finals.end()) {
				finals.insert(epsTransState);
			}

			for (auto symbol : epsDest->getSymbols()) {
				for (auto dest : epsDest->getTransitions(symbol)) {
					epsTransState->setTransition(symbol, dest);
				}
			}

			epsTransState->deleteTransition(State::EPS, epsDest);

			if ( ! isAccessible(epsDest)) {
				states.erase(epsDest);
				finals.erase(epsDest);
				delete epsDest;
			}
		}
	}
/*
	if ( ! isAccessible(final) && finals.find(final) != finals.end()) {
		finals.erase(final);
		delete final;
	}
*/
}

unordered_set<State*> Nfa::getFinals() {
	return finals;
}

bool Nfa::isAccessible(State* state) {
	unordered_set<State*> accessible_states;
	for (auto& current_state : states) {
		for (auto s : current_state->getSymbols()) {
			accessible_states = current_state->getTransitions(s);
			if (accessible_states.find(state) != accessible_states.end())
				return true;
		}
	}
	return false;
}

State * Nfa::getStateWithEpsTransitions() {
	//TODO Return only the first state
	for (auto state : states) {
		if ( ! state->getTransitions(State::EPS).empty()) {
			return state;
		}
	}
	return NULL;
}

void Nfa::applyCardinality(enum card_t type) {
	State *newInitial = new State();
	State *newFinal = new State();
	states.insert(newInitial);
	states.insert(newFinal);

	State *final = *finals.begin();

	final->setTransition(State::EPS, newFinal);
	newInitial->setTransition(State::EPS, initial);

	if (type != OPTION)
		final->setTransition(State::EPS, initial);

	if (type != PLUS)
		newInitial->setTransition(State::EPS, newFinal);

	initial = newInitial;
	finals.clear();
	finals.insert(newFinal);
}

void Nfa::applyCardinality(int min, int max) {
	Nfa orig = Nfa(this);
	State *newFinal = new State();

	int i;
	for (i = 1; i < min; i++) {
		Nfa toAppend = Nfa(orig);
		concatenate(&toAppend);
	}

	switch (max) {
		case SIMPLE_COUNT:
			break;	//do_nothing
		case UNBOUNDED: {
			Nfa toAppend = Nfa(orig);
			toAppend.applyCardinality(KLEENE_STAR);
			concatenate(&toAppend);
			break;
		}
		default: {
			State *final = *finals.begin();
			forward_list<State *> abortStates;
			abortStates.push_front(final);

			for (; i < max; i++) {
				Nfa toAppend = Nfa(orig);
				concatenate(&toAppend);
				abortStates.push_front(final);
			}

			for (auto& abort : abortStates) {
				(*states.find(abort))->setTransition(State::EPS, newFinal);
			}
			states.insert(newFinal);
			finals.clear();
			finals.insert(newFinal);
		}
	}
}

unordered_set<State *> Nfa::getStates() {
	return states;
}

State * Nfa::getInitial() {
	return initial;
}

void Nfa::print() {
	cout << "Initial: " << initial->getId() << ", final: ";

	for (auto& f : finals) {
		cout << f->getId() << " , ";
	}
	cout << "\n";

	for (auto& state : states) {
		state->print();
	}
	cout << "\n\n";
}

void Nfa::toDot(char const *fileName) {
	FILE *dotFile = fopen(fileName, "w");
	fprintf(dotFile, "digraph nfa{\n");

	initial->toDot(dotFile, "color=green");

	for (auto& state : finals)
		state->toDot(dotFile, "peripheries=2");


	for (auto& state : states) {
		if (state == initial || finals.find(state) != finals.end())
			continue;
		state->toDot(dotFile, "");
	}
	fprintf(dotFile, "}");
	fclose(dotFile);
}
