/*
 * Tree.h
 *
 *  Created on: 04/apr/2015
 *      Author: daniele
 */

#ifndef SRC_TREE_H_
#define SRC_TREE_H_

#include <cstddef>
#include "State.h"

class Tree {
public:
	Tree(State *);
	Tree(Tree *, Tree *);
	virtual ~Tree();

	void setSkip();
	void applyRules();
	void print();

private:
	bool skip;
	State *state;
	Tree *left;
	Tree *right;
};

#endif /* SRC_TREE_H_ */
