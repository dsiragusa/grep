/*
 * State.cpp
 *
 *  Created on: 28/mar/2015
 *      Author: Daniele
 */

#include "State.h"

UniqueIdGenerator State::idGen = UniqueIdGenerator(0);
const int State::EPS = 0x00000100;
const int State::DOT = 0x00000200;

State::State() {
	id = idGen.getUniqueId();
}

void State::adaptTransitions(const State *toCopy, const map<State *, State *> *oldToNewStates) {
	for (auto& trans : toCopy->transitions) {
		unordered_set<State *> newNextStates;
		for (auto& nextState : trans.second) {
			newNextStates.insert(oldToNewStates->find(nextState)->second);
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

unordered_set<State *> State::getTransitions(int symbol) {
	auto states = transitions.find(symbol);
	if (states == transitions.end())
		return unordered_set<State *>();

	return states->second;
}

list<int> State::get_symbols() {
	list<int> symbols;
	for(auto& t:transitions){
		symbols.push_back(t.first);
	}
	return	symbols;
}
void State::setTransition(int symbol, State *state) {
	if (transitions.find(symbol) == transitions.end())
		transitions.emplace(symbol, unordered_set<State *>());

	transitions.find(symbol)->second.insert(state);
}

void State::print() {
	cout << getId() << " -> ";
	for (auto& tr : transitions) {
		int sy = tr.first;
		cout << getChar(sy) << "(";
		for (auto& next : tr.second) {
			cout << next->getId() << " ";
		}
		cout << ") ";
	}
	cout << "\n";
}

void State::delete_transition(int symbol,State* toDelete) {
	auto transition = transitions.find(symbol);
	if (transition == transitions.end())
		return;

	transition->second.erase(toDelete);
	if (transition->second.empty())
		transitions.erase(symbol);
}

string State::getChar(int symbol) {
	switch (symbol) {
	case EPS:
		return "EPS";
	case DOT:
		return "DOT";
	default:
		return string(1, symbol);
	}
}
