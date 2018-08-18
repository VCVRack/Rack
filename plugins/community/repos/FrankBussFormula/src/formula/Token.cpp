/**
 *
 * Copyright (c) 2001, Frank Buﬂ
 *
 * project: Formula
 * version: $Revision: 1.3 $ $Name:  $
 *
 * All token classes, used by the Parser class.
 */

#include "Token.h"
#include <iostream>


void OperatorToken::eval(Parser& parser)
{
	// precondition
	Token* nextToken = parser.peekNextToken();
	if (!dynamic_cast<IdentifierToken*>(nextToken) &&
	        !dynamic_cast<OpenBracketToken*>(nextToken) &&
	        !dynamic_cast<NumberToken*>(nextToken) &&
	        !dynamic_cast<NotToken*>(nextToken) &&
	        !dynamic_cast<SubToken*>(nextToken))
	{
		throw SyntaxError("Expecting a variable, function, '(', number, not or negate operator.");
	}

	// eval
	while (parser.m_operators.size() > 0 && parser.m_operators.top()->getPrecedence() >= getPrecedence()) {
		parser.m_postfix += " ";
		parser.m_postfix += parser.m_operators.top()->getValue();
		Token* t = parser.m_operators.top();
		if (dynamic_cast<OperatorToken*>(t)) parser.m_evaluator.addAction(((OperatorToken*)(t))->getAction());
		parser.m_operators.pop();
	}
	parser.m_operators.push(this);
	parser.skipToken();
}


void NumberToken::eval(Parser& parser)
{
	// precondition
	Token* nextToken = parser.peekNextToken();
	if (dynamic_cast<NumberToken*>(nextToken) || dynamic_cast<IdentifierToken*>(nextToken))
	{
		throw SyntaxError("One after another number is not allowed.");
	}

	// eval
	parser.m_postfix += " ";
	parser.m_postfix += m_value;
	parser.m_evaluator.addAction(new NumberAction(m_value));
	parser.skipToken();
}

void CommaToken::eval(Parser& parser)
{
	// precondition
	Token* nextToken = parser.peekNextToken();
	if (!dynamic_cast<IdentifierToken*>(nextToken) &&
	        !dynamic_cast<OpenBracketToken*>(nextToken) &&
	        !dynamic_cast<NumberToken*>(nextToken) &&
	        !dynamic_cast<NotToken*>(nextToken) &&
	        !dynamic_cast<SubToken*>(nextToken))
	{
		throw SyntaxError("Expecting a variable, function, '(', number, not or negate operator.");
	}

	// eval
	while (parser.m_operators.size() > 0 && parser.m_operators.top()->getPrecedence() >= getPrecedence() &&
	        !( dynamic_cast<OpenBracketToken*>(parser.m_operators.top())
	           || dynamic_cast<IdentifierToken*>(parser.m_operators.top()) ))
	{
		parser.m_postfix += " ";
		parser.m_postfix += parser.m_operators.top()->getValue();
		Token* t = parser.m_operators.top();
		if (dynamic_cast<OperatorToken*>(t)) parser.m_evaluator.addAction(((OperatorToken*)(t))->getAction());
		parser.m_operators.pop();
	}
	if (parser.m_functionArgumentCountStack.size() == 0) throw SyntaxError("',' is allowed within functions only.");
	parser.m_functionArgumentCountStack.top()++;
	parser.skipToken();
}

void OpenBracketToken::eval(Parser& parser)
{
	// precondition
	Token* nextToken = parser.peekNextToken();
	if (!dynamic_cast<IdentifierToken*>(nextToken) &&
	        !dynamic_cast<OpenBracketToken*>(nextToken) &&
	        !dynamic_cast<CloseBracketToken*>(nextToken) &&
	        !dynamic_cast<NumberToken*>(nextToken) &&
	        !dynamic_cast<NotToken*>(nextToken) &&
	        !dynamic_cast<SubToken*>(nextToken))
	{
		throw SyntaxError("Expecting a variable, function, '(', ')', number, not or negate operator.");
	}

	// eval
	parser.m_operators.push(this);
	parser.skipToken();
}

void IdentifierToken::eval(Parser& parser)
{
	// precondition
	Token* nextToken = parser.peekNextToken();
	if (dynamic_cast<NumberToken*>(nextToken) || dynamic_cast<IdentifierToken*>(nextToken))
	{
		throw SyntaxError("One after another number is not allowed.");
	}

	// eval
	parser.skipToken();
	Token* t = parser.peekToken();
	if (dynamic_cast<OpenBracketToken*>(t))
	{
		// function, skip '(' and push this token; "this" will be used at ')'
		parser.skipToken();

		// test, if this is a function without an argument
		if (dynamic_cast<CloseBracketToken*>(parser.peekToken())) {
			parser.m_evaluator.addAction(new NoArgumentFunctionAction(&parser.m_evaluator, parser.getNoArgumentFunction(m_value)));
			// skip ')'
			parser.skipToken();
		} else {
			parser.m_operators.push(this);
			parser.m_functionArgumentCountStack.push(1);
		}
	} else
	{
		// variable
		parser.m_postfix += " ";
		parser.m_postfix += m_value;
		parser.m_evaluator.addAction(new VariableAction(&parser.m_evaluator, m_value));
	}
}


void CloseBracketToken::eval(Parser& parser)
{
	// precondition
	Token* nextToken = parser.peekNextToken();
	if (nextToken && !dynamic_cast<CloseBracketToken*>(nextToken) && !dynamic_cast<OperatorToken*>(nextToken) && !dynamic_cast<CommaToken*>(nextToken))
	{
		throw SyntaxError("Expected ')', ',' or operator after ')'.");
	}

	// eval
	while (parser.m_operators.size() > 0 &&
	        !( dynamic_cast<OpenBracketToken*>(parser.m_operators.top())
	           || dynamic_cast<IdentifierToken*>(parser.m_operators.top()) ))
	{
		Token* t = parser.m_operators.top();
		parser.m_postfix += " ";
		parser.m_postfix += t->getValue();
		if (dynamic_cast<OperatorToken*>(t)) {
			parser.m_evaluator.addAction(((OperatorToken*)t)->getAction());
		} else {
			throw SyntaxError("')' found but there is no matching '('.");
		}
		parser.m_operators.pop();
	}
	if (parser.m_operators.size() == 0) throw SyntaxError("')' found but there is no matching '('.");
	Token* t = parser.m_operators.top();
	if (dynamic_cast<IdentifierToken*>(t)) {
		if (parser.m_functionArgumentCountStack.size() == 0) throw SyntaxError("')' found but there is no matching '('.");
		int argCount = parser.m_functionArgumentCountStack.top();
		parser.m_functionArgumentCountStack.pop();
		string functionName = parser.m_operators.top()->getValue();
		parser.m_postfix += " ";
		parser.m_postfix += functionName;
		switch (argCount) {
		case 1:
			parser.m_evaluator.addAction(new OneArgumentFunctionAction(&parser.m_evaluator, parser.getOneArgumentFunction(functionName)));
			break;
		case 2:
			parser.m_evaluator.addAction(new TwoArgumentsFunctionAction(&parser.m_evaluator, parser.getTwoArgumentsFunction(functionName)));
			break;
		default:
			throw TooManyArgumentsError(functionName);
		}
	}
	parser.m_operators.pop();
	parser.skipToken();
}


void SubToken::eval(Parser& parser)
{
	// precondition is in OperatorToken::eval(parser)

	// eval
	Token* lastToken = parser.peekLastToken();
	if (dynamic_cast<NumberToken*>(lastToken)
	        || dynamic_cast<IdentifierToken*>(lastToken)
	        || dynamic_cast<CloseBracketToken*>(lastToken))
	{
		m_action = new SubAction();
		m_precedence = AddSubPrecedence;
	} else {
		m_value = "neg";
		m_action = new NegAction();
		m_precedence = NegPrecedence;
	}
	OperatorToken::eval(parser);
}


void AddToken::eval(Parser& parser)
{
	// precondition is in OperatorToken::eval(parser)

	// eval
	Token* lastToken = parser.peekLastToken();
	if (dynamic_cast<NumberToken*>(lastToken)
	        || dynamic_cast<IdentifierToken*>(lastToken)
	        || dynamic_cast<CloseBracketToken*>(lastToken))
	{
		OperatorToken::eval(parser);
	} else
	{
		// skip unary '+' operator
		parser.skipToken();
	}
}
