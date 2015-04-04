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

void Dfa::rec_determinize(Nfa *toDeterminize,
		map<set<State *>, State *> *superSet, set<State *> currentId) {
	if (superSet->find(currentId) != superSet->end())
		return;

	State *current = new State();
	superSet->emplace(currentId, current);
	states.insert(current);

	map<int, set<State *> > transitions;

	unordered_set<State *> dotTrans;
	for (auto state : currentId) {
		auto dot = state->getTransitions(State::DOT);
		dotTrans.insert(dot.begin(), dot.end());
	}

	for (auto state : currentId) {
		if (toDeterminize->getFinals().find(state)
				!= toDeterminize->getFinals().end())
			finals.insert(current);

		for (auto symbol : state->get_symbols()) {
			auto nextStates = state->getTransitions(symbol);

			if (transitions.find(symbol) == transitions.end()) {
				auto tmp = set<State *>(nextStates.begin(), nextStates.end());
				transitions.emplace(symbol, tmp);
			} else {
				transitions.find(symbol)->second.insert(nextStates.begin(),
						nextStates.end());
			}

			transitions.find(symbol)->second.insert(dotTrans.begin(),
					dotTrans.end());
		}
	}

	for (auto transition : transitions) {
		rec_determinize(toDeterminize, superSet, transition.second);
		current->setTransition(transition.first,
				superSet->find(transition.second)->second);
	}
}

void Dfa::print_finals() {
	cout << ", final:  ";
	for (auto& f : finals) {
		cout << f->getId() << " , ";
	}
	cout << "\n";
}

void Dfa::print() {
	cout << "Initial: " << initial->getId();
	print_finals();
	for (auto& state : states) {
		state->print();
	}
	cout << "\n\n";
}

unordered_set<int> Dfa::get_symbols() {
	unordered_set<int> symbols;
	list<int> states_sysmbol;
	for (auto& state : states) {
		states_sysmbol = state->get_symbols();
		for (auto s : states_sysmbol) {
			if (symbols.find(s) == symbols.end()) {
				// Does not exists
				symbols.insert(s);
			}
		}
	}
	return symbols;
}

int Dfa::evaluate(string in) {
	int result = rec_evaluate_second(in, initial);
	cout << "\n" << in << ": " << ((result == ACCEPT) ? "YES" : "NO") << "\n\n";
	return result;
}

int Dfa::rec_evaluate(string in, State *state, State* final) {
	if (in.length() == 0) {
		cout << "empty string: state " << state->getId() << "\n";
		if (state == final)
			return ACCEPT;

		for (auto& eps_state : state->getTransitions(State::EPS))
			if (rec_evaluate(in, eps_state,final) == ACCEPT)
				return ACCEPT;

		return REJECT;
	}

	for (auto& next_state : state->getTransitions(in.at(0)))
		if (rec_evaluate(in.substr(1), next_state,final) == ACCEPT)
			return ACCEPT;

	for (auto& eps_state : state->getTransitions(State::EPS))
		if (rec_evaluate(in, eps_state,final) == ACCEPT)
			return ACCEPT;

	for (auto& next_state : state->getTransitions(State::DOT))
		if (rec_evaluate(in.substr(1), next_state,final) == ACCEPT)
			return ACCEPT;

	return REJECT;
}

int Dfa::rec_evaluate_second(string word, State* state) {
		for (auto& f : finals) {
			if (rec_evaluate(word, state,f) == ACCEPT)
				return ACCEPT;
		}
		return REJECT;
}
/*
 * 
 * 
 * 
 let reachable_states:= {q0};
 let new_states:= {q0};
 do {
 temp := the empty set;
 for each q in new_states do
 for all c in Σ do
 temp := temp ∪ {p such that p=δ(q,c)};
 end;
 end;
 new_states := temp \ reachable_states;
 reachable_states := reachable_states ∪ new_states;
 } while(new_states ≠ the empty set);
 unreachable_states := Q \ reachable_states;
 **/
unordered_set<State *> difference(unordered_set<State *> first,
		unordered_set<State *> second) {
	unordered_set<State *> result = unordered_set<State *>();
	bool exist;
	for (auto& f : first) {
		exist = false;
		for (auto& s : second) {
			if (f == s) {
				exist = true;
				break;
			}
		}
		if (!exist) {
			result.insert(f);
		}
	}
	return result;
}

unordered_set<State *> Union(unordered_set<State *> first,
		unordered_set<State *> second) {
	unordered_set<State *> result = unordered_set<State *>(first);
	bool exist;
	for (auto& s : second) {
		exist = false;
		for (auto& r : result) {
			if (r == s) {
				exist = true;
				break;
			}
		}
		if (!exist) {
			result.insert(s);
		}
	}
	return result;
}

unordered_set<State *> Intersection(unordered_set<State *> first,
		unordered_set<State *> second) {
	unordered_set<State *> result = unordered_set<State *>();
	bool exist;
	for (auto& s : second) {
		exist = false;
		for (auto& f : first) {
			if (f == s) {
				exist = true;
				break;
			}
		}
		if (exist) {
			result.insert(s);
		}
	}
	return result;
}

unordered_set<State*> Dfa::unreachable_states() {
	unordered_set<State *> reachable_states;
	unordered_set<State *> new_states;
	reachable_states.insert(initial);
	new_states.insert(initial);
	unordered_set<State *> temp;
	unordered_set<int> symbols = get_symbols();
	unordered_set<State*> _P;
	State* P;
	do {
		temp = unordered_set<State*>();
		for (auto& q : new_states) {
			for (auto s : symbols) {
				_P = q->getTransitions(s);
				if (!_P.empty()) {
					P = *_P.begin();
					if (temp.find(P) == temp.end()) {
						temp.insert(P);
					}
				}
			}
		}
		new_states = difference(temp, reachable_states);
		reachable_states = Union(reachable_states, new_states);
	} while (temp != new_states);
	return difference(states, reachable_states);
}

bool canTransitToStateOn(State* current, int symbol, unordered_set<State*> B) {
	unordered_set<State*> reachable_states = current->getTransitions(symbol);
	for (auto& state : reachable_states) {
		if (B.find(state) != B.end()) {
			return true;
		}
	}
	return false;
}

/*
 *
 *
 P := {F, Q \ F};
 W := {F};
 while (W is not empty) do
 choose and remove a set A from W
 for each c in Σ do
 let X be the set of states for which a transition on c leads to a state in A
 for each set Y in P for which X ∩ Y is nonempty and Y \ X is nonempty do
 replace Y in P by the two sets X ∩ Y and Y \ X
 if Y is in W
 replace Y in W by the same two sets
 else
 if |X ∩ Y| <= |Y \ X|
 add X ∩ Y to W
 else
 add Y \ X to W
 end;
 end;
 end;
 *
 *
 */

bool equals(unordered_set<State*> first, unordered_set<State*> second) {
	if (first.size() != second.size()) {
		return false;
	} else {
		bool exist;
		for (auto& f : first) {
			exist = false;
			for (auto& s : second) {
				if (s == f) {
					exist = true;
					break;
				}
			}
			if (!exist) {
				return false;
			}
		}
		return true;
	}
}

bool contains(list<unordered_set<State*>> liste, unordered_set<State*> Set) {
	if (liste.empty())
		return false;
	for (auto& s : liste) {
		if (equals(s, Set))
			return true;
	}
	return false;
}

void Dfa::minimise_hopcroft() {
	if (isMinimal)
		return;
	else {
		unordered_set<State*> notFinal = difference(states, finals);
		list<unordered_set<State*> > P;
		list<unordered_set<State*> > __P;
		list<unordered_set<State*> > W;
		P.push_front(finals);
		P.push_front(notFinal);
		W.push_back(finals);
		unordered_set<int> symbols = get_symbols();
		unordered_set<State*> temp;
		unordered_set<State*> intersection;
		unordered_set<State*> diff;

		unordered_set<State*> setX;
		while (!W.empty()) {
			temp = W.front();
			W.pop_front();

			for (auto& s : symbols) {
				unordered_set<State*> setX = unordered_set<State*>();
				for (auto& current : states) {
					if (canTransitToStateOn(current, s, temp)) {
						setX.insert(current);
					}
				}
				__P = list<unordered_set<State*> >(P);
				for (auto& setY : __P) {
					intersection = Intersection(setX, setY);
					diff = difference(setY, setX);
					if (!intersection.empty() && !diff.empty()) {
						P.remove(setY);
						P.push_front(intersection);
						P.push_front(diff);

						if (contains(W, setY)) {
							W.remove(setY);
							W.push_back(intersection);
							W.push_back(diff);
						} else {
							if (intersection.size() <= diff.size()) {
								W.push_back(intersection);
							} else {
								W.push_back(diff);
							}
						}
					}
				}
			}
		}
		initializeStatesAfterMinimisation(P);
	}
	isMinimal = true;
}

void Dfa::initializeStatesAfterMinimisation(
		list<unordered_set<State*> > _list) {

	int size = _list.size();
	State* result[size];
	unordered_set<State*> setOFfinals;
	if (size > 0) {
		State* state;
		for (int i = 0; i < size; i++) {
			result[i] = new State();
		}

		auto list_temp = list<unordered_set<State*> >();
		for (auto& currentSet : _list) {
			list_temp.push_back(currentSet);
		}

		bool initialFound = false;
		unordered_set<State*>::iterator it;
		map<int, unordered_set<State *> > old_trans;
		int counter = 0;
		for (auto& currentSet : _list) {
			if (currentSet.size() > 0) {
				state = result[counter++];
			}
			if (!initialFound) {
				auto init = currentSet.find(initial);
				if (init != currentSet.end()) {
					initialFound = true;
					initial= state;
				}
			}
			if (currentSet.size() > 0) {
				it = currentSet.begin();
				auto s = *it;
				old_trans = s->getTransitions();
				if (finals.find(s) != finals.end()) {
					if (setOFfinals.find(state) == setOFfinals.end()) {
						setOFfinals.insert(state);
					}
				}
				for (auto& transition : old_trans) {
					auto exist = true;
					auto rank = 0;
					auto symbol = transition.first;
					auto nextState = *(transition.second.begin());
					for (auto& nextSet : list_temp) {
						if (nextSet.find(nextState) != nextSet.end()) {
							auto temp = result[rank];
							state->setTransition(symbol, temp);
						}
						rank++;
					}
				}
			}
		}
		unordered_set<State*> setOfresults;
		for (int i = 0; i < size; i++) {
			setOfresults.insert(result[i]);
		}
		states = setOfresults;
		finals = setOFfinals;
	}
}
/*
 int main() {
 Nfa *a = new Nfa('a');
 Nfa *b = new Nfa('b');
 Nfa *c = new Nfa('c');

 Nfa *d = new Nfa('d');
 Nfa *eps = new Nfa(State::EPS);
 Nfa *e = new Nfa('e');

 b->concatenate(c);
 a->concatenate(b);
 a->startAnywhere();
 a->endAnywhere();
 a->concatenate(c);
 a->concatenate(a);
 a->concatenate(eps);
 a->concatenate(b);
 a->concatenate(d);
 a->concatenate(eps);
 a->concatenate(b);
 a->concatenate(e);

 a->concatenate(eps);
 a->concatenate(b);
 a->concatenate(d);
 a->concatenate(eps);
 a->startAnywhere();

 a->print();

 a->eliminate_eps();

 a->print();

 Dfa *dfa = new Dfa(a);
 dfa->print();
 dfa->minimise_hopcroft();
 dfa->print();
 }*/
