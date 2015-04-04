/*
 * Tree.cpp
 *
 *  Created on: 04/apr/2015
 *      Author: daniele
 */

#include "Tree.h"

Tree::Tree(State *final) {
	cout << "creating new leaf: state" << final->getId() << endl;
	state = final;
	left = NULL;
	right = NULL;
	lineEnd = false;
}

Tree::Tree(Tree *l, Tree *r) {
	cout << "creating new node with l: ";
	if (l->state != NULL)
		cout << l->state->getId();
	else
		cout << "NODE";
	cout << " r: ";
	if (r->state != NULL)
		cout << r->state->getId();
	else
		cout << "NODE";
	cout << endl;
	state = NULL;
	left = l;
	right = r;
	lineEnd = false;

}

void Tree::setLineEnd() {
	lineEnd = true;
}

void Tree::applyEndRules() {
	cout << "applying end rules";

	if (state != NULL)
		cout << " for state " << state->getId();
	else
		cout << " empty node";

	if (lineEnd) {
		cout << " $ detected" << endl;
		return;
	}

	if (state != NULL) {
		cout << " applying DOT" << endl;
		state->setTransition(State::DOT, state);
	}
	else {
		cout << endl;
		left->applyEndRules();
		right->applyEndRules();
	}
}

Tree::~Tree() {
	// TODO Auto-generated destructor stub
}

