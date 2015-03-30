#include <iostream>
#include <map>
#include <list>

using namespace std;
#define ACCEPT 1
#define REJECT 0

enum card_t {KLEENE_STAR, OPTION, PLUS};

class nfa {
	private:
	static const char EPS;
	static int state_counter;
	
	nfa(const nfa *);
	map<int, map<char,list<int> > > transitions;
	int final;
	int initial;
	
	list<int> get_transitions(int, char);
	int rec_evaluate(string, int);
	
	public:
	nfa(char);
	~nfa();
	void concatenate(const nfa);
	void unify(const nfa);
	void apply_cardinality(enum card_t);
	void apply_cardinality(int, int);
	
	void print();
	int evaluate(string);
};
