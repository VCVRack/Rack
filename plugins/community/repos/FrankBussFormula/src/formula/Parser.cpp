/**
 *
 * Copyright (c) 2001, Frank Buﬂ
 *
 * project: Formula
 * version: $Revision: 1.3 $ $Name:  $
 *
 * Parser class.
 */

#include "Token.h"
#include "Parser.h"

#include <math.h>
#include <iostream>
#include <time.h>


float ParserMax(float argument1, float argument2)
{
	return argument1 > argument2 ? argument1 : argument2;
}

float ParserMin(float argument1, float argument2)
{
	return argument1 < argument2 ? argument1 : argument2;
}

float ParserTime()
{
	return time(NULL);
}

Parser::Parser(string expression)
{
	setFunction("acos", acosf);
	setFunction("asin", asinf);
	setFunction("atan", atanf);
	setFunction("atan2", atan2f);
	setFunction("cos", cosf);
	setFunction("cosh", coshf);
	setFunction("exp", expf);
	setFunction("abs", fabsf);
	setFunction("mod", fmodf);
	setFunction("log", logf);
	setFunction("log2", log2f);
	setFunction("log10", log10f);
	setFunction("pow", powf);
	setFunction("sin", sinf);
	setFunction("sinh", sinhf);
	setFunction("tan", tanf);
	setFunction("tanh", tanhf);
	setFunction("sqrt", sqrtf);
	setFunction("ceil", ceilf);
	setFunction("floor", floorf);
	setFunction("max", ParserMax);
	setFunction("min", ParserMin);

	setExpression(expression);
}


Parser::~Parser()
{
	deleteTokens();
}


char Parser::peekChar()
{
	if (m_currentIndex < (int) m_expression.size()) return m_expression[m_currentIndex];
	return 0;
}


void Parser::skipChar()
{
	m_currentIndex++;
}


char Parser::skipAndPeekChar()
{
	skipChar();
	return peekChar();
}


Token* Parser::peekToken()
{
	if (m_currentTokenIndex < (int) m_tokens.size()) return m_tokens[m_currentTokenIndex];
	return NULL;
}


Token* Parser::peekLastToken()
{
	Token* lastToken = NULL;
	if (m_currentTokenIndex > 0) lastToken = m_tokens[m_currentTokenIndex - 1];
	return lastToken;
}


Token* Parser::peekNextToken()
{
	Token* nextToken = NULL;
	if (m_currentTokenIndex + 1 < (int) m_tokens.size()) nextToken = m_tokens[m_currentTokenIndex + 1];
	return nextToken;
}


void Parser::skipToken()
{
	m_currentTokenIndex++;
}


string Parser::parseNumber(char c)
{
	string number;
	if (c != '.')
	{
		// parse before '.'
		while (c != 0 && c >= '0' && c <= '9' && c != '.' && c != 'e' && c != 'E') {
			number += c;
			c = skipAndPeekChar();
		}
	}
	if (c == '.')
	{
		// parse after '.'
		number += c;
		c = skipAndPeekChar();
		if (c != 0 && c >= '0' && c <= '9') {
			while (c != 0 && c >= '0' && c <= '9' && c != 'e' && c != 'E') {
				number += c;
				c = skipAndPeekChar();
			}
		} else {
			throw SyntaxError("Expected digit after '.', number: " + number);
		}
	}
	if (c == 'e' || c == 'E')
	{
		// parse after 'e' or 'E'
		number += c;
		c = skipAndPeekChar();
		if (c == '+' || c == '-') {
			number += c;
			c = skipAndPeekChar();
		}
		while (c != 0 && c >= '0' && c <= '9') {
			number += c;
			c = skipAndPeekChar();
		}
	}
	return number;
}


string Parser::parseIdentifier(char c)
{
	string identifier;
	identifier += c;
	c = skipAndPeekChar();
	while (c != 0 && ((c >= 'a' && c <= 'z')
	                  || (c >= 'A' && c <= 'Z')
	                  || (c >= '0' && c <= '9')
	                  || c == '_'))
	{
		identifier += c;
		c = skipAndPeekChar();
	}
	return identifier;
}


void Parser::deleteTokens()
{
	for (int i = 0; i < (int) m_tokens.size(); i++) delete m_tokens[i];
	m_tokens.clear();
}


void Parser::setExpression(string expression)
{
	m_expression = string("(") + expression + ")";

	m_postfix = "";
	m_evaluator.removeAllActions();
	m_functionArgumentCountStack = stack<int>();
	m_operators = stack<Token*>();
	deleteTokens();

	m_currentIndex = 0;
	char c;
	Token* token;
	while ((c = peekChar())) {
		token = NULL;
		switch (c) {
		case '&':
			token = new AndToken();
			skipChar();
			break;
		case '|':
			token = new OrToken();
			skipChar();
			break;
		case '=':
			token = new EqualToken();
			skipChar();
			break;
		case '!':
			skipChar();
			if (peekChar() == '=') {
				skipChar();
				token = new NotEqualToken();
			} else {
				token = new NotToken();
			}
			break;
		case '<':
			skipChar();
			if (peekChar() == '=') {
				skipChar();
				token = new LessEqualToken();
			} else {
				token = new LessToken();
			}
			break;
		case '>':
			skipChar();
			if (peekChar() == '=') {
				skipChar();
				token = new GreaterEqualToken();
			} else {
				token = new GreaterToken();
			}
			break;
		case '+':
			token = new AddToken();
			skipChar();
			break;
		case '-':
			token = new SubToken();
			skipChar();
			break;
		case '*':
			token = new MulToken();
			skipChar();
			break;
		case '/':
			token = new DivToken();
			skipChar();
			break;
		case '^':
			token = new PowerToken();
			skipChar();
			break;
		case '(':
			token = new OpenBracketToken();
			skipChar();
			break;
		case ')':
			token = new CloseBracketToken();
			skipChar();
			break;
		case ',':
			token = new CommaToken();
			skipChar();
			break;
		default:
			if ((c >= '0' && c <= '9') || c == '.') {
				token = new NumberToken(parseNumber(c));
			} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
				token = new IdentifierToken(parseIdentifier(c));
			} else if (c == 9 || c == 10 || c == 13 || c == 32) {
				skipChar();
				continue;
			} else {
				skipChar();
				throw SyntaxError(string("Invalid character: ") + c);
			}
		}
		if (token) m_tokens.push_back(token);
	}

	m_currentTokenIndex = 0;
	while ((token = peekToken())) token->eval(*this);
	if (m_operators.size() > 0) throw SyntaxError("Missing ')'.");
	if (m_postfix.size() > 0) m_postfix = m_postfix.substr(1);
}

void Parser::setFunction(string name, float(*function)())
{
	m_noArgumentFunctions[name] = function;
}

void Parser::setFunction(string name, float(*function)(float))
{
	m_oneArgumentFunctions[name] = function;
}

void Parser::setFunction(string name, float(*function)(float, float))
{
	m_twoArgumentsFunctions[name] = function;
}

NoArgumentFunction Parser::getNoArgumentFunction(string name)
{
	NoArgumentFunction function = m_noArgumentFunctions[name];
	if (function) {
		return function;
	} else {
		throw FunctionNotFound(name);
	}
}

OneArgumentFunction Parser::getOneArgumentFunction(string name)
{
	OneArgumentFunction function = m_oneArgumentFunctions[name];
	if (function) {
		return function;
	} else {
		throw FunctionNotFound(name);
	}
}

TwoArgumentsFunction Parser::getTwoArgumentsFunction(string name)
{
	TwoArgumentsFunction function = m_twoArgumentsFunctions[name];
	if (function) {
		return function;
	} else {
		throw FunctionNotFound(name);
	}
}
