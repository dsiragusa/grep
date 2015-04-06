/*
 * BracketClassApplier.h
 *
 *  Created on: 06/apr/2015
 *      Author: daniele
 */

#ifndef SRC_BRACKETEXPRMANAGER_H_
#define SRC_BRACKETEXPRMANAGER_H_

#include <cstdlib>
#include <string>
#include <map>
#include <forward_list>
using namespace std;


class BracketExprManager {
public:
	BracketExprManager();
	virtual ~BracketExprManager();
	bool expandRange();
	void applyClass(char *);
	void setCollElem(char);
	forward_list<int> getBracket();
	void clear();

private:
	typedef map<string, void(BracketExprManager::*)()> funMap;
	funMap mapClassToApply;
	forward_list<int> bracket;
	void applyLower();
	void applyUpper();
	void applyAlpha();
	void applyDigit();
	void applyAlnum();
	void applyAscii();
	void applyBlank();
	void applyCntrl();
	void applyGraph();
	void applyPrint();
	void applyPunct();
	void applySpace();
	void applyWord();
	void applyXdigit();
};

#endif /* SRC_BRACKETEXPRMANAGER_H_ */
