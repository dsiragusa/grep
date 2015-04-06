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
	skip = false;
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
	skip = false;

}

void Tree::setSkip() {
	skip = true;
}

void Tree::applyRules() {
	if (state != NULL)
		cout << " for state " << state->getId();
	else
		cout << " empty node";

	if (skip) {
		cout << " skip detected" << endl;
		return;
	}

	if (state != NULL) {
		cout << " applying DOT" << endl;
		state->setTransition(State::DOT, state);
	}
	else {
		cout << endl;
		left->applyRules();
		right->applyRules();
	}
}

void Tree::print() {
	if (state == NULL)
		cout << "Node" << endl;
	else
		cout << "Leaf " << state->getId() << endl;
}

Tree::~Tree() {
	if (left != NULL)
		delete left;
	if (right != NULL)
		delete right;
}

