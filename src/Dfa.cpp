/*
 * Dfa.cpp
 *
 *  Created on: 31/mar/2015
 *      Author: daniele
 */

#include "Dfa.h"
#include "Nfa.h"

Dfa::Dfa(Nfa *toDeterminize) {
	determinize(toDeterminize);
}

Dfa::~Dfa() {
	for (auto state : states)
		delete state;
}

void Dfa::determinize(Nfa *toDeterminize) {
	map<set<State *>, State *> *superSet = new map<set<State *>, State *>();

	set<State *> initialId;
	initialId.insert(toDeterminize->getInitial());

	recDeterminize(toDeterminize, superSet, initialId);
	initial = superSet->find(initialId)->second;
	delete superSet;
}

void Dfa::recDeterminize(Nfa *toDeterminize,
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
		if (toDeterminize->getFinals().find(state) != toDeterminize->getFinals().end())
			finals.insert(current);

		for (auto symbol : state->getSymbols()) {
			auto nextStates = state->getTransitions(symbol);

			if (transitions.find(symbol) == transitions.end()) {
				auto tmp = set<State *>(nextStates.begin(), nextStates.end());
				transitions.emplace(symbol, tmp);
			} else {
				transitions[symbol].insert(nextStates.begin(), nextStates.end());
			}

			transitions[symbol].insert(dotTrans.begin(), dotTrans.end());
		}
	}

	for (auto transition : transitions) {
		recDeterminize(toDeterminize, superSet, transition.second);
		current->setTransition(transition.first,
				superSet->find(transition.second)->second);
	}
}

void Dfa::print() {
	cout << "Initial: " << initial->getId() << ", final:  ";

	for (auto& f : finals)
		cout << f->getId() << " , ";

	cout << "\n";

	for (auto& state : states)
		state->print();

	cout << "\n\n";
}

unordered_set<int> Dfa::getSymbols() {
	unordered_set<int> symbols;
	for (auto& state : states) {
		forward_list<int> stateSymbols = state->getSymbols();
		symbols.insert(stateSymbols.begin(), stateSymbols.end());
	}
	return symbols;
}

int Dfa::evaluate(string in) {
	int result = recEvaluate(in, initial);
	cout << "\n" << in << ": " << ((result == ACCEPT) ? "YES" : "NO") << "\n\n";
	return result;
}

int Dfa::recEvaluate(string in, State *state) {
	if (in.length() == 0)
		return (finals.find(state) != finals.end()) ? ACCEPT : REJECT;

	for (auto& next_state : state->getTransitions(in.at(0)))
		if (recEvaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;

	if (state->getTransitions(in.at(0)).empty())
		for (auto& next_state : state->getTransitions(State::DOT))
			if (recEvaluate(in.substr(1), next_state) == ACCEPT)
				return ACCEPT;

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

bool canTransitToStateOn(State* current, int symbol, unordered_set<State*> B) {
	unordered_set<State*> reachable_states = current->getTransitions(symbol);
	for (auto& state : reachable_states) {
		if (B.find(state) != B.end()) {
			return true;
		}
	}
	return false;
}

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
		unordered_set<int> symbols = getSymbols();
		unordered_set<State*> temp;
		unordered_set<State*> intersection;
		unordered_set<State*> diff;

		unordered_set<State*> setX;
		while (!W.empty()) {
			temp = W.front();
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
			W.pop_front();
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
					initial = state;
				}
			}
			if (currentSet.size() > 0) {
				it = currentSet.begin();
				auto s = *it;
				old_trans = s->getAllTransitions();
				if (finals.find(s) != finals.end()) {
					if (setOFfinals.find(state) == setOFfinals.end()) {
						setOFfinals.insert(state);
					}
				}

				for (auto& transition : old_trans) {
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

void Dfa::minimize() {
	Nfa *transposed = new Nfa(this);

	states.clear();
	finals.clear();
	determinize(transposed);
	delete transposed;

	transposed = new Nfa(this);

	states.clear();
	finals.clear();

	determinize(transposed);

	delete transposed;
}

unordered_set<State *> Dfa::getStates() {
	return states;
}

State * Dfa::getInitial() {
	return initial;
}

unordered_set<State *> Dfa::getFinals() {
	return finals;
}

void Dfa::toDot(char const *fileName) {
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
