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
	if (skip)
		return;

	if (state != NULL)
		state->setTransition(State::DOT, state);
	else {
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

