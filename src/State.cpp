/*
 * State.cpp
 *
 *  Created on: 28/mar/2015
 *      Author: daniele
 */

#include "State.h"

UniqueIdGenerator State::idGen = UniqueIdGenerator(0);
const char State::EPS = '\0';

State::State() {
	id = idGen.getUniqueId();
}

void State::adaptTransitions(const State *toCopy, const map<State *, State *> *oldToNewStates) {
	for (auto& trans : toCopy->transitions) {
		list<State *> newNextStates;
		for (auto& nextState : trans.second) {
			newNextStates.push_back(oldToNewStates->find(nextState)->second);
		}

		transitions.emplace(trans.first, newNextStates);
	}
}

void State::copyTransitions(const State *toCopy) {
	transitions.insert(toCopy->transitions.begin(), toCopy->transitions.end());
}

State::~State() {
	// TODO Auto-generated destructor stub
}

int State::getId() {
	return id;
}

list<State *> State::getTransitions(char symbol) {
	auto states = transitions.find(symbol);
	if (states == transitions.end())
		return list<State *>();

	return states->second;
}

list<State *> State::getEpsTransitions() {
	return getTransitions(EPS);
}

void State::setTransition(char symbol, State *state) {
	if (transitions.find(symbol) == transitions.end())
		transitions.emplace(symbol, list<State *>());

	transitions.find(symbol)->second.push_back(state);
}

void State::setEpsTransition(State *state) {
	setTransition(EPS, state);
}

void State::print() {
	cout << getId() << " -> ";
	for (auto& tr : transitions) {
		char sy = tr.first;
		cout << ((sy == EPS) ? '@' : sy) << "(";
		for (auto& next : tr.second) {
			cout << next->getId();
		}
		cout << ") ";
	}
	cout << "\n";
}
