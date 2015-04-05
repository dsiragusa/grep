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
		auto newState = oldToNewStates.find(*stateToCopy)->second;
		newState->adaptTransitions(*stateToCopy, &oldToNewStates);

		stateToCopy++;
	}

	initial = oldToNewStates.find(toCopy->initial)->second;
	final = oldToNewStates.find(toCopy->final)->second;
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

void Nfa::eliminate_eps() {
	unordered_set<State*> _states;
	unordered_set<State*> _states1;
	list<State*> _stateEPS = getStatesWithEpSTransition();
	State* current;
	list<int> symbols;

	finals.insert(final);

	while (!_stateEPS.empty()) {
		current = _stateEPS.front();
		_stateEPS.pop_front();
		_states = current->getTransitions(State::EPS);
		for (auto& s : _states) {
			symbols = s->get_symbols();
			if (finals.find(s)!=finals.end()) {
				if (finals.find(current) == finals.end()) {
					finals.insert(current);
				}
			}
			for (auto symbol : symbols) {
				_states1 = s->getTransitions(symbol);
				// Create a transitions
				for (auto e1 : _states1) {
					current->setTransition(symbol, e1);
				}
			}
			current->delete_transition(State::EPS, s);
			if (!isAccessible(s)) {
				states.erase(s);
				finals.erase(s);
			}
		}
		//states.insert(current);
		_stateEPS = getStatesWithEpSTransition();
	}
	if (!isAccessible(final)) {
		finals.erase(final);
	}
}

unordered_set<State*> Nfa::getFinals() {
	return finals;
}

bool Nfa::isAccessible(State* state) {
	list<int> symbols;
	unordered_set<State*> accessible_states;
	for (auto& current_state : states) {
		symbols = current_state->get_symbols();
		for (auto s : symbols) {
			accessible_states = current_state->getTransitions(s);
			if (accessible_states.find(state) != accessible_states.end())
				return true;
		}
	}
	return false;
}

list<State*> Nfa::getStatesWithEpSTransition() {
	unordered_set<State*> temp;
	list<State*> result = list<State*>();
	for (auto state : states) {
		temp = state->getTransitions(State::EPS);
		if (!temp.empty()) {
			result.push_back(state);
		}
	}
	return result;
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

		for (auto& eps_state : state->getTransitions(State::EPS))
			if (rec_evaluate(in, eps_state) == ACCEPT)
				return ACCEPT;

		return REJECT;
	}

	bool symbolHasTrans = false;
	for (auto& next_state : state->getTransitions(in.at(0))) {
		symbolHasTrans = true;
		if (rec_evaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;
	}

	for (auto& eps_state : state->getTransitions(State::EPS))
		if (rec_evaluate(in, eps_state) == ACCEPT)
			return ACCEPT;

	if (symbolHasTrans)
		return REJECT;

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

int Nfa::rec_evaluate_second(string word, State* state) {
	if (finals.size() > 0) {
		for (auto& f : finals) {
			final = f;
			if (rec_evaluate(word, state) == ACCEPT)
				return ACCEPT;
		}
		return REJECT;
	} else
		return rec_evaluate(word, state);
}

void Nfa::print_finals() {
	cout << ", final:  ";
	if (finals.size() > 0) {
		for (auto& f : finals) {
			cout << f->getId() << " , ";
		}
		cout << "\n";
	} else {
		cout << final->getId() << "\n";
	}
}

int Nfa::evaluate(string in) {
	int result = rec_evaluate_second(in, initial);
	cout << "\n" << in << ": " << ((result == ACCEPT) ? "YES" : "NO") << "\n\n";
	return result;
}

void Nfa::print() {
	cout << "Initial: " << initial->getId();
	print_finals();
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

/*
 int main() {
 Nfa *a = new Nfa('a');
 Nfa *b = new Nfa('b');
 Nfa *c = new Nfa('c');
 Nfa *d = new Nfa('d');
 a->concatenate(b);
 a->concatenate(c);
 a->concatenate(d);
 a->toDot("test.dot");
 a->print();
 Nfa *e = new Nfa(a);
 e->print();
 a->toDot("copy.dot");
 //a->apply_cardinality(2, 4);
 }
 */
