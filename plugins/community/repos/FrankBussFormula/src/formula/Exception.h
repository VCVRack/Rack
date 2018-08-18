/**
 *
 * Copyright (c) 2001, Frank Buﬂ
 *
 * project: Formula
 * version: $Revision: 1.3 $ $Name:  $
 *
 * Exception classes for the formula system.
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H


#include <exception>
#include <string>
#include <sstream>

#define EXCEPTION_UNKNOWNFILENAME        "<unknown>"
#define EXCEPTION_UNKNOWNLINENUMBER      (-1)

using namespace std;


class ParserException : public exception
{
public:
	ParserException(string message = "");
	ParserException(const ParserException& other);
	virtual ~ParserException();
	ParserException& operator= (const ParserException& other);
	const char *what() const throw () override;
	string getMessage() {
		return m_message;
	}

protected:
	string m_message;
};

// Construct the exception
inline ParserException::ParserException(const ParserException& other)
	: exception(other)
{
	m_message = other.m_message;
}

inline ParserException::ParserException(string message)
	: m_message(message)
{}


// Destruct the exception
inline ParserException::~ParserException()
{}


// Perform an assignment
inline ParserException& ParserException::operator= (const ParserException& other)
{
	exception::operator= (other);
	if (&other != this) m_message = other.m_message;
	return *this;
}


// Return descriptive message
inline const char *ParserException::what() const throw ()
{
	return m_message.c_str();
}



class SyntaxError : public ParserException
{
public:
	explicit SyntaxError(string message) : ParserException(message) {}
};

class EvalError: public ParserException
{
public:
	explicit EvalError(string message) : ParserException(message) {}
};

class VariableNotFound : public EvalError
{
public:
	explicit VariableNotFound(string variableName) :
		EvalError("Variable not found: " + variableName), m_variableName(variableName) {}
	string getVariableName();
private:
	string m_variableName;
};


inline string VariableNotFound::getVariableName()
{
	return m_variableName;
}

class FunctionNotFound : public EvalError
{
public:
	explicit FunctionNotFound(string functionName) :
		EvalError("Function not found: " + functionName), m_functionName(functionName) {}
	string getFunctionName();
private:
	string m_functionName;
};


inline string FunctionNotFound::getFunctionName()
{
	return m_functionName;
}

class TooManyArgumentsError : public EvalError
{
public:
	explicit TooManyArgumentsError(string functionName) :
		EvalError("Too many arguments for function: " + functionName), m_functionName(functionName) {}
	string getFunctionName();
private:
	string m_functionName;
};


inline string TooManyArgumentsError::getFunctionName()
{
	return m_functionName;
}


class StackUnderflow : public EvalError
{
public:
	explicit StackUnderflow() : EvalError("Stack underflow. Check formula syntax.") {}
};


class MathError : public EvalError
{
public:
	explicit MathError() : EvalError("Illegal calculation (division by zero, sqrt(-1) etc.).") {}
};


#endif
