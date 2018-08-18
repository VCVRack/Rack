/**
 *
 * Copyright (c) 2001, Frank Buß
 *
 * project: Formula
 * version: $Revision: 1.3 $ $Name:  $
 *
 * The Formula class is the facade class for accessing the fomula system.
 */

#include "Formula.h"
#include "Parser.h"

Formula::Formula()
{
	m_parser = new Parser("");
}

Formula::Formula(string expression)
{
	m_parser = new Parser(expression);
}

Formula::~Formula()
{
	delete m_parser;
}

void Formula::setExpression(string expression)
{
	m_parser->setExpression(expression);
}


void Formula::setVariable(string name, float value)
{
	m_parser->setVariable(name, value);
}


float* Formula::getVariableAddress(string name)
{
	return m_parser->getVariableAddress(name);
}


void Formula::setFunction(string name, float(*function)())
{
	m_parser->setFunction(name, function);
}



void Formula::setFunction(string name, float(*function)(float))
{
	m_parser->setFunction(name, function);
}



void Formula::setFunction(string name, float(*function)(float, float))
{
	m_parser->setFunction(name, function);
}



float Formula::eval()
{
	return m_parser->eval();
}
