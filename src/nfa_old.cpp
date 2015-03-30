#include "nfa.hpp"

int nfa::state_counter = 0;
const char nfa::EPS = '\0';

nfa::nfa(char symbol){
	//create a new nfa with 2 states and a transition between them with the specified symbol
	initial = state_counter++;
	final = state_counter++;
	
	list<int> to_final;
	to_final.push_back(final);
	
	map<char, list<int> > initial_trans;
	initial_trans.emplace(symbol, to_final);
	
	transitions.emplace(initial, initial_trans);
	transitions.emplace(final, map<char, list<int> >());
}

nfa::nfa(const nfa *to_copy) {
	map<int, int> old_to_new_states;

	for (auto& trans : to_copy->transitions) {
		int new_state = state_counter++;
		old_to_new_states.emplace(trans.first, new_state);
	}
	
	for (auto& trans : to_copy->transitions) {
		map<char, list<int> > new_trans;
		for (auto& next_states : trans.second) {
			list<int> new_next_states;
			for (auto& next_state : next_states.second) {
				new_next_states.push_back(old_to_new_states.find(next_state)->second);
			}
			new_trans.emplace(next_states.first, new_next_states);
		}
		transitions.emplace(old_to_new_states.find(trans.first)->second, new_trans);
	}
		
	initial = old_to_new_states.find(to_copy->initial)->second;
	final = old_to_new_states.find(to_copy->final)->second;
}

nfa::~nfa () {

}
 
void nfa::concatenate (const nfa next) {
	//copy all of next's transitions minus the initial one
	transitions.insert(next.transitions.begin(), next.transitions.end());
	transitions.erase(next.initial);
	
	//find all edges departing from next's initial state and copy them to the current final state
	transitions.find(final)->second = next.transitions.find(next.initial)->second;
	
	final = next.final;	
}

void nfa::unify(const nfa to_unify) {
	int new_initial = state_counter++;
	int new_final = state_counter++;
	
	//copy all the other's nfa transitions into this one
	transitions.insert(to_unify.transitions.begin(), to_unify.transitions.end());
	
	//create new initial state with e-transitions to the two nfa's initial states
	list<int> eps_to_initials;
	eps_to_initials.push_back(initial);
	eps_to_initials.push_back(to_unify.initial);
	
	map<char, list<int> > trans_to_initials;
	trans_to_initials.emplace(EPS, eps_to_initials);
	
	transitions.emplace(new_initial, trans_to_initials);
	
	//create new final state with entering e-transitions from the two nfa's final states
	map<char, list<int> > trans_to_final;
	list<int> eps_to_new_final;
	
	eps_to_new_final.push_back(new_final);
	trans_to_final.emplace(EPS, eps_to_new_final);
	
	transitions.find(final)->second = trans_to_final;
	transitions.find(to_unify.final)->second = trans_to_final;
	
	transitions.emplace(new_final, map<char, list<int> >());
	
	//add new initial and final states to nfa
	initial = new_initial;
	final = new_final;
}

void nfa::apply_cardinality(enum card_t type) {
	int new_initial = state_counter++;
	int new_final = state_counter++;

	//create e-transitions to initial and new_final
	list<int> eps_edges;
	
	eps_edges.push_back(new_final);
	switch (type) {
		case PLUS:
		case KLEENE_STAR:
			eps_edges.push_back(initial);
			transitions.find(final)->second.emplace(EPS, eps_edges);
			break;
		case OPTION:
			//add it to final
			transitions.find(final)->second.emplace(EPS, eps_edges);
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

void nfa::apply_cardinality(int min, int max) {
	nfa orig = nfa(this);
	int new_final = state_counter++;
	list<int> to_new_final;
	to_new_final.push_back(new_final);
	
	int i;
	for (i = 1; i < min; i++) {
		concatenate(nfa(&orig));
	}
	
	list<int> abort_states;
	abort_states.push_back(final);
	
	for (; i < max; i++) {
		concatenate(nfa(&orig));
		abort_states.push_back(final);
	}
	
	for (auto& abort : abort_states) {
	 	transitions.find(abort)->second.emplace(EPS, to_new_final);	
	}
	
	transitions.emplace(new_final, map<char, list<int> >());
	final = new_final;
}

list<int> nfa::get_transitions(int state, char symbol) {
	auto trans = transitions.find(state);
	if (trans == transitions.end())
		return list<int>();
	
	auto next_states = trans->second.find(symbol);
	if (next_states == trans->second.end())
		return list<int>();
		
	return next_states->second;
}

void nfa::print() {
	cout << "Initial: " << initial << ", final: " << final << "\n";
	for (auto& tr : transitions) {
		cout << tr.first << " -> ";
		for (auto& sy : tr.second) {
			cout << sy.first << " (";
			for (auto& nxt : sy.second) {
				cout << nxt << " ";
			}
			cout << ") ";
		}
		cout << "\n";
	}
	cout << "\n\n";
}

int nfa::evaluate(string in) {
	int result = rec_evaluate(in, initial);
	cout << "\n" << in << ": " << ((result == ACCEPT) ? "YES" : "NO") << "\n\n"; 
	return result;
}

int nfa::rec_evaluate(string in, int state) {
	if (in.length() == 0) {
		cout << "empty string: state " << state << "\n";
		if (state == final)
			return ACCEPT;

		list<int> eps_states = get_transitions(state, EPS);
		for (auto& eps_state : eps_states)
			if (rec_evaluate(in, eps_state) == ACCEPT)
				return ACCEPT;
			
		return REJECT;
	}
	
	list<int> next_states = get_transitions(state, in.at(0));
	list<int> eps_states = get_transitions(state, EPS);
		
	for (auto& next_state : next_states)
		if (rec_evaluate(in.substr(1), next_state) == ACCEPT)
			return ACCEPT;
	
	for (auto& eps_state : eps_states)
		if (rec_evaluate(in, eps_state) == ACCEPT)
			return ACCEPT;
	
	return REJECT;
}

int main () {
	nfa a = nfa('a');
	a.print();
	a.evaluate("a");
	
	nfa b = nfa('b');
	b.print();
	b.evaluate("bb"); 
	
	//ab
 	a.concatenate(b);	
	a.print();
 	a.evaluate("ab"); 
	
	//(ab|b)
//  	a.unify(b);
// 	a.evaluate("a");
// 	a.evaluate("b");
// 	a.evaluate("aaaabbaab");

	//(ab|b)*
//  	a.apply_cardinality(KLEENE_STAR);
//  	a.print();
//  	a.evaluate("");
//  	a.evaluate("a");
//  	a.evaluate("ab");	
//  	a.evaluate("abab");	
//  	a.evaluate("bbbabbbbabbbb");	
	a.apply_cardinality(3, 5);
	a.print();
	a.evaluate("ab");
	a.evaluate("abab");
	a.evaluate("ababab");
	a.evaluate("abababab");
	a.evaluate("ababababab");
	a.evaluate("abababababab");
	
	nfa c = nfa('b');
	a.unify(c);
	a.print();
	a.evaluate("b");
	a.evaluate("ababababab");
}