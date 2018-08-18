/**
 *
 * Copyright (c) 2001, Frank Buﬂ
 *
 * project: Formula
 * version: $Revision: 1.3 $ $Name:  $
 *
 * Parser class.
 */

#ifndef PARSER_H
#define PARSER_H

#include "Evaluator.h"
#include <string>
#include <vector>
#include <stack>

using namespace std;

class Token;

class Parser
{
	friend class OperatorToken;
	friend class AddToken;
	friend class SubToken;
	friend class NumberToken;
	friend class OpenBracketToken;
	friend class CloseBracketToken;
	friend class IdentifierToken;
	friend class CommaToken;

public:
	Parser(string expression);
	~Parser();
	void setExpression(string expression);
	void setVariable(string name, float value) {
		m_evaluator.setVariable(name, value);
	}
	float* getVariableAddress(string name) {
		return m_evaluator.getVariableAddress(name);
	}
	void setFunction(string name, NoArgumentFunction function);
	void setFunction(string name, OneArgumentFunction function);
	void setFunction(string name, TwoArgumentsFunction function);
	NoArgumentFunction getNoArgumentFunction(string name);
	OneArgumentFunction getOneArgumentFunction(string name);
	TwoArgumentsFunction getTwoArgumentsFunction(string name);
	
	string getPostfix() {
		return m_postfix;
	}
	float eval() {
		return m_evaluator.eval();
	}


private:
	void deleteTokens();
	string parseNumber(char c);
	string parseIdentifier(char c);
	char peekChar();
	void skipChar();
	char skipAndPeekChar();
	Token* peekToken();
	Token* peekNextToken();
	Token* peekLastToken();
	void skipToken();

	string m_expression;
	int m_currentIndex;
	int m_currentTokenIndex;
	string m_postfix;
	Evaluator m_evaluator;
	stack<Token*> m_operators;
	vector<Token*> m_tokens;
	stack<int> m_functionArgumentCountStack;
	map<string, NoArgumentFunction> m_noArgumentFunctions;
	map<string, OneArgumentFunction> m_oneArgumentFunctions;
	map<string, TwoArgumentsFunction> m_twoArgumentsFunctions;
};


#endif
