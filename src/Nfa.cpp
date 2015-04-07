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
	final = new State();
	states.insert(final);

	initial->setTransition(symbol, final);
}

Nfa::Nfa(forward_list<int> symbols, bool areMatching) {
	initial = new State();
	states.insert(initial);
	final = new State();
	states.insert(final);

	for (auto& symbol : symbols) {
		initial->setTransition(symbol, final);
	}

	if ( ! areMatching) {
		final = new State();
		states.insert(final);
		initial->setTransition(State::DOT, final);
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
	final = oldToNewStates[toCopy->final];
}

Nfa::Nfa(Dfa *toTranspose) {
	map<State *, State *> mapStates;

	for (auto dfaState : toTranspose->getStates()) {
		State *newState = new State();
		states.insert(newState);
		mapStates.emplace(dfaState, newState);
	}

	final = mapStates[toTranspose->getInitial()];

	for (auto dfaState : toTranspose->getStates()) {
		for (auto trans : dfaState->getAllTransitions()) {
			State *dest = *(trans.second.begin());
			State *newDest = mapStates[dest];
			newDest->setTransition(trans.first, mapStates[dfaState]);
		}
	}

	if (toTranspose->getFinals().size() > 1) {
		initial = new State();
		for (auto dfaFinal : toTranspose->getFinals())
			initial->setTransition(State::EPS, mapStates[dfaFinal]);
		states.insert(initial);

		this->toDot("beforeEps.dot");
		eliminateEps();
		this->toDot("afterEps.dot");
	}
	else {
		State *dfaFinal = *(toTranspose->getFinals().begin());
		initial = mapStates[dfaFinal];
		finals.insert(final);
	}
}

Nfa::~Nfa() {
	for (auto state : states)
		delete state;
}

void Nfa::concatenate(const Nfa *toAppend) {
	states.insert(toAppend->states.begin(), toAppend->states.end());
	states.erase(toAppend->initial);
	(*states.find(final))->copyTransitions(toAppend->initial);
	delete toAppend->initial;

	final = toAppend->final;
}

void Nfa::unify(const Nfa *toUnify) {
	State *newInitial = new State();
	State *newFinal = new State();
	states.insert(toUnify->states.begin(), toUnify->states.end());
	states.insert(newInitial);
	states.insert(newFinal);

	newInitial->setTransition(State::EPS, initial);
	newInitial->setTransition(State::EPS, toUnify->initial);

	final->setTransition(State::EPS, newFinal);
	toUnify->final->setTransition(State::EPS, newFinal);

	initial = newInitial;
	final = newFinal;
}

void Nfa::eliminateEps() {
	State * epsTransState;

	finals.insert(final);

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

	if ( ! isAccessible(final) && finals.find(final) != finals.end()) {
		finals.erase(final);
		delete final;
	}
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

	final->setTransition(State::EPS, newFinal);
	newInitial->setTransition(State::EPS, initial);

	if (type != OPTION)
		final->setTransition(State::EPS, initial);

	if (type != PLUS)
		newInitial->setTransition(State::EPS, newFinal);

	initial = newInitial;
	final = newFinal;
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
			final = newFinal;
		}
	}
}

int Nfa::recEvaluate(string in, State *state) {
	if (in.length() == 0) {
		if (finals.find(state) != finals.end())
			return ACCEPT;

		for (auto& eps_state : state->getTransitions(State::EPS))
			if (recEvaluate(in, eps_state) == ACCEPT)
				return ACCEPT;

		return REJECT;
	}

	bool symbolHasTrans = false;
	for (auto& next_state : state->getTransitions(in.at(0))) {
		symbolHasTrans = true;
		if (recEvaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;
	}

	for (auto& eps_state : state->getTransitions(State::EPS))
		if (recEvaluate(in, eps_state) == ACCEPT)
			return ACCEPT;

	if (symbolHasTrans)
		return REJECT;

	for (auto& next_state : state->getTransitions(State::DOT))
		if (recEvaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;

	return REJECT;
}

unordered_set<State *> Nfa::getStates() {
	return states;
}

State * Nfa::getInitial() {
	return initial;
}

State * Nfa::getFinal() {
	return final;
}

int Nfa::evaluate(string in) {
	bool emptyFinals = finals.empty();

	if (emptyFinals)
		finals.insert(final);

	int result = recEvaluate(in, initial);
	cout << "\n" << in << ": " << ((result == ACCEPT) ? "YES" : "NO") << "\n\n";

	if (emptyFinals)
		finals.clear();

	return result;
}

void Nfa::print() {
	cout << "Initial: " << initial->getId() << ", final: ";

	if (finals.size() > 0) {
		for (auto& f : finals) {
			cout << f->getId() << " , ";
		}
		cout << "\n";
	}
	else {
		cout << final->getId() << "\n";
	}

	for (auto& state : states) {
		state->print();
	}
	cout << "\n\n";
}

void Nfa::toDot(char const *fileName) {
	FILE *dotFile = fopen(fileName, "w");
	fprintf(dotFile, "digraph nfa{\n");
	for (auto& state : states) {
		state->toDot(dotFile);
	}
	fprintf(dotFile, "}");
	fclose(dotFile);
}
