/*
 * BracketClassApplier.cpp
 *
 *  Created on: 06/apr/2015
 *      Author: daniele
 */

#include "BracketExprManager.h"

BracketExprManager::BracketExprManager() {
	mapClassToApply.insert(funMap::value_type("alnum", &BracketExprManager::applyAlnum));
	mapClassToApply.insert(funMap::value_type("alpha", &BracketExprManager::applyAlpha));
	mapClassToApply.insert(funMap::value_type("ascii", &BracketExprManager::applyAscii));
	mapClassToApply.insert(funMap::value_type("blank", &BracketExprManager::applyBlank));
	mapClassToApply.insert(funMap::value_type("cntrl", &BracketExprManager::applyCntrl));
	mapClassToApply.insert(funMap::value_type("digit", &BracketExprManager::applyDigit));
	mapClassToApply.insert(funMap::value_type("graph", &BracketExprManager::applyGraph));
	mapClassToApply.insert(funMap::value_type("lower", &BracketExprManager::applyLower));
	mapClassToApply.insert(funMap::value_type("print", &BracketExprManager::applyPrint));
	mapClassToApply.insert(funMap::value_type("punct", &BracketExprManager::applyPunct));
	mapClassToApply.insert(funMap::value_type("space", &BracketExprManager::applySpace));
	mapClassToApply.insert(funMap::value_type("upper", &BracketExprManager::applyUpper));
	mapClassToApply.insert(funMap::value_type("word", &BracketExprManager::applyWord));
	mapClassToApply.insert(funMap::value_type("xdigit", &BracketExprManager::applyXdigit));
}

BracketExprManager::~BracketExprManager() {
	// TODO Auto-generated destructor stub
}

bool BracketExprManager::expandRange() {
	int end = bracket.front();
	bracket.pop_front();
	int start = bracket.front();

	if (end < start)
		return false;

	for (int i = start + 1; i <= end; i++) {
		bracket.push_front(i);
	}

	return true;
}

void BracketExprManager::applyLower() {
	bracket.push_front('a');
	bracket.push_front('z');
	expandRange();
}

void BracketExprManager::applyUpper() {
	bracket.push_front('A');
	bracket.push_front('Z');
	expandRange();
}

void BracketExprManager::applyAlpha() {
	applyLower();
	applyUpper();
}

void BracketExprManager::applyDigit() {
	bracket.push_front('0');
	bracket.push_front('9');
	expandRange();
}

void BracketExprManager::applyAlnum() {
	applyDigit();
	applyAlpha();
}

void BracketExprManager::applyAscii() {
	bracket.push_front(0x0);
	bracket.push_front(0x7f);
	expandRange();
}

void BracketExprManager::applyBlank() {
	bracket.push_front(' ');
	bracket.push_front('\t');
}

void BracketExprManager::applyCntrl() {
	bracket.push_front(0x0);
	bracket.push_front(0x1f);
	expandRange();
	bracket.push_front(0x7f);
}

void BracketExprManager::applyGraph() {
	bracket.push_front(0x21);
	bracket.push_front(0x7e);
	expandRange();
}

void BracketExprManager::applyPrint() {
	bracket.push_front(0x20);
	applyGraph();
}

void BracketExprManager::applyPunct() {
	bracket.push_front(0x21);
	bracket.push_front(0x2f);
	expandRange();
	bracket.push_front(0x3a);
	bracket.push_front(0x40);
	expandRange();
	bracket.push_front(0x5b);
	bracket.push_front(0x60);
	expandRange();
	bracket.push_front(0x7b);
	bracket.push_front(0x7e);
	expandRange();
}

void BracketExprManager::applySpace() {
	applyBlank();
	bracket.push_front('\r');
	bracket.push_front('\n');
	bracket.push_front('\v');
	bracket.push_front('\f');
}

void BracketExprManager::applyWord() {
	applyAlnum();
	bracket.push_front('_');
}

void BracketExprManager::applyXdigit() {
	applyDigit();
	bracket.push_front('a');
	bracket.push_front('f');
	expandRange();
	bracket.push_front('A');
	bracket.push_front('F');
	expandRange();
}

void BracketExprManager::applyClass(char *clazz) {
	string cl_name = string(clazz);
	void(BracketExprManager::*apply)() = mapClassToApply[cl_name];
	(this->*apply)();
}

forward_list<int> BracketExprManager::getBracket() {
	return bracket;
}

void BracketExprManager::setCollElem(char collElem) {
	bracket.push_front(collElem);
}

void BracketExprManager::clear() {
	bracket.clear();
}
