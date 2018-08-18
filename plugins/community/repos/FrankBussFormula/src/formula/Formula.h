/**
 *
 * Copyright (c) 2001, Frank Buﬂ
 *
 * project: Formula
 * version: $Revision: 1.3 $ $Name:  $
 *
 * The Formula class is the facade class for accessing the fomula system.
 */

#ifndef FORMULA_H
#define FORMULA_H

#include "Exception.h"

using namespace std;

class Parser;

class Formula
{
public:
	Formula();
	Formula(string formula);
	~Formula();
	void setExpression(string expression);
	void setVariable(string name, float value);
	float* getVariableAddress(string name);
	void setFunction(string name, float(*function)());
	void setFunction(string name, float(*function)(float));
	void setFunction(string name, float(*function)(float, float));
	float eval();

private:
	Parser* m_parser;
};


#endif
