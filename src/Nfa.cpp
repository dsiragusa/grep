/*
 * Nfa.cpp
 *
 *  Created on: 28/mar/2015
 *      Author: daniele
 */

#include "Nfa.h"

Nfa::Nfa(int symbol) {
	initial = new State();
	states.insert(initial);
	final = new State();
	states.insert(final);

	initial->setTransition(symbol, final);
}

Nfa::Nfa(list<int> symbols, bool areNegated) {
	initial = new State();
	states.insert(initial);
	final = new State();
	states.insert(final);

	for (auto& symbol : symbols) {
		initial->setTransition(symbol, final);
	}

	if (areNegated) {
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
	auto myState = states.begin();

	while (stateToCopy != toCopy->states.end()) {
		(*myState)->adaptTransitions(*stateToCopy, &oldToNewStates);

		stateToCopy++;
		myState++;
	}

	initial = oldToNewStates.find(toCopy->initial)->second;
	final = oldToNewStates.find(toCopy->final)->second;
}

//should be called whenever $ is not present
void Nfa::startAnywhere() {
	initial->setTransition(State::DOT, initial);
}

//should be called whenever ^ is not present
void Nfa::endAnywhere() {
	final->setTransition(State::DOT, final);
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

	newInitial->setTransition(State::EPS, initial);
	newInitial->setTransition(State::EPS, toUnify->initial);

	final->setTransition(State::EPS, newFinal);
	toUnify->final->setTransition(State::EPS, newFinal);

	initial = newInitial;
	final = newFinal;
}

void Nfa::apply_cardinality(enum card_t type) {
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

void Nfa::apply_cardinality(int min, int max) {
	Nfa *orig = new Nfa(this);
	State *newFinal = new State();

	int i;
	for (i = 1; i < min; i++) {
		Nfa *toAppend = new Nfa(orig);
		concatenate(toAppend);
	}

	switch (max) {
		case SIMPLE_COUNT:
			break;	//do_nothing
		case UNBOUNDED: {
			Nfa *toAppend = new Nfa(orig);
			toAppend->apply_cardinality(KLEENE_STAR);
			concatenate(toAppend);
			break;
		}
		default: {
			list<State *> abortStates;
			abortStates.push_back(final);

			for (; i < max; i++) {
				Nfa *toAppend = new Nfa(orig);
				concatenate(toAppend);
				abortStates.push_back(final);
			}

			for (auto& abort : abortStates) {
				(*states.find(abort))->setTransition(State::EPS, newFinal);
			}
			states.insert(newFinal);
			final = newFinal;
		}
	}
}

int Nfa::rec_evaluate(string in, State *state) {
	if (in.length() == 0) {
		cout << "empty string: state " << state->getId() << "\n";
		if (state == final)
			return ACCEPT;

		list<State *> eps_states = state->getTransitions(State::EPS);
		for (auto& eps_state : eps_states)
			if (rec_evaluate(in, eps_state) == ACCEPT)
				return ACCEPT;

		return REJECT;
	}

	list<State *> next_states = state->getTransitions(in.at(0));
	list<State *> eps_states = state->getTransitions(State::EPS);

	for (auto& next_state : next_states)
		if (rec_evaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;


	for (auto& eps_state : eps_states)
		if (rec_evaluate(in, eps_state) == ACCEPT)
			return ACCEPT;

	for (auto& next_state : state->getTransitions(State::DOT))
		if (rec_evaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;

	return REJECT;
}

Nfa::~Nfa() {
	// TODO Auto-generated destructor stub
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
	int result = rec_evaluate(in, initial);
	cout << "\n" << in << ": " << ((result == ACCEPT) ? "YES" : "NO") << "\n\n";
	return result;
}

void Nfa::print() {
	cout << "Initial: " << initial->getId() << ", final: " << final->getId() << "\n";
	for (auto& state : states) {
		state->print();
	}
	cout << "\n\n";
}


int main () {

	/*
	Nfa *a = new Nfa('a');
	a->print();
	a->evaluate("a");

	Nfa *b = new Nfa('b');
	b->print();
	b->evaluate("b");
	b->evaluate("a");

	//a->unify(b);
	a->concatenate(b);
	a->print();
	a->evaluate("a");
	a->evaluate("b");
	a->evaluate("ab");


	a->apply_cardinality(3, 5);
	a->print();
	a->evaluate("ab");
	a->evaluate("abab");
	a->evaluate("ababab");
	a->evaluate("abababab");
	a->evaluate("ababababab");
	a->evaluate("abababababab");
	*/

/*
	list<int> chars;
	chars.push_back('a');
	chars.push_back('b');
	chars.push_back('c');
	Nfa *a = new Nfa(chars, false);
	a->endAnywhere();
	a->startAnywhere();

	Nfa *b = new Nfa('d');
	Nfa *c = new Nfa('e');
	b->unify(c);
	b->apply_cardinality(PLUS);
	b->endAnywhere();

	a->unify(b);
	a->print();
	a->evaluate("a");
	a->evaluate("b");
	a->evaluate("c");
	a->evaluate("abc");
	a->evaluate("d");
	a->evaluate("dedede");
	a->evaluate("fdedede");
*/
	set<int> listA, listB, listC, listD;
	listA.insert(1); listA.insert(2); listA.insert(3);
	listB.insert(2); listB.insert(1); listB.insert(3);
	listC.insert(1); listC.insert(2); listC.insert(4);
	listD.insert(1); listD.insert(3); listD.insert(4);


	map<set<int>, string> myMap;
	myMap.emplace(listA, "toto");
	myMap.emplace(listC, "soleil");

	cout << myMap.find(listB)->second << endl;
}
