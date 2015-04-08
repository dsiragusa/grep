/*
 * Tree.cpp
 *
 *  Created on: 04/apr/2015
 *      Author: daniele
 */

#include "Tree.h"

Tree::Tree(State *final) {
	state = final;
	left = NULL;
	right = NULL;
	skip = false;
}

Tree::Tree(Tree *l, Tree *r) {
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

Tree::~Tree() {
	if (left != NULL)
		delete left;
	if (right != NULL)
		delete right;
}

