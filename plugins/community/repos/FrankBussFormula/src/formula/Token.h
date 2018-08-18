/**
 *
 * Copyright (c) 2001, Frank Buﬂ
 *
 * project: Formula
 * version: $Revision: 1.3 $ $Name:  $
 *
 * All token classes, used by the Parser class.
 */

#include "Parser.h"
#include "Evaluator.h"

enum Precedences {
	LowestPrecedence,
	OrPrecedence,
	AndPrecedence,
	EqualPrecedence,
	RelationalPrecedence,
	AddSubPrecedence,
	MulDivPrecedence,
	NegPrecedence,
	NotPrecedence,
	PowerPrecedence
};

class Token
{
public:
	Token(string value) : m_value(value) {}
	virtual ~Token() {}
	virtual void eval(Parser& parser) = 0;
	string getValue() {
		return m_value;
	}
	virtual int getPrecedence() {
		return LowestPrecedence;
	}
protected:
	string m_value;
};

class OperatorToken : public Token
{
public:
	OperatorToken(string value) : Token(value) {}
	virtual void eval(Parser& parser) override;
	virtual Action* getAction() {
		return NULL;
	}
};

class AddToken : public OperatorToken
{
public:
	AddToken() : OperatorToken("+") {}
	virtual void eval(Parser& parser) override;
	virtual Action* getAction() override {
		return new AddAction();
	}
	virtual int getPrecedence() override {
		return AddSubPrecedence;
	}
};

class SubToken : public OperatorToken
{
public:
	SubToken() : OperatorToken("-") {}
	virtual void eval(Parser& parser) override;
	virtual Action* getAction() override {
		return m_action;
	}
	virtual int getPrecedence() override {
		return m_precedence;
	}
private:
	Action* m_action;
	int m_precedence;
};

class MulToken : public OperatorToken
{
public:
	MulToken() : OperatorToken("*") {}
	virtual Action* getAction() override {
		return new MulAction();
	}
	virtual int getPrecedence() override {
		return MulDivPrecedence;
	}
};

class DivToken : public OperatorToken
{
public:
	DivToken() : OperatorToken("/") {}
	virtual Action* getAction() override {
		return new DivAction();
	}
	virtual int getPrecedence() override {
		return MulDivPrecedence;
	}
};

class PowerToken : public OperatorToken
{
public:
	PowerToken() : OperatorToken("^") {}
	virtual Action* getAction() override {
		return new PowerAction();
	}
	virtual int getPrecedence() override {
		return PowerPrecedence;
	}
};


class LessToken : public OperatorToken
{
public:
	LessToken() : OperatorToken("<") {}
	virtual Action* getAction() override {
		return new LessAction();
	}
	virtual int getPrecedence() override {
		return RelationalPrecedence;
	}
};


class GreaterToken : public OperatorToken
{
public:
	GreaterToken() : OperatorToken(">") {}
	virtual Action* getAction() override {
		return new GreaterAction();
	}
	virtual int getPrecedence() override {
		return RelationalPrecedence;
	}
};


class LessEqualToken : public OperatorToken
{
public:
	LessEqualToken() : OperatorToken("<=") {}
	virtual Action* getAction() override {
		return new LessEqualAction();
	}
	virtual int getPrecedence() override {
		return RelationalPrecedence;
	}
};


class GreaterEqualToken : public OperatorToken
{
public:
	GreaterEqualToken() : OperatorToken(">=") {}
	virtual Action* getAction() override {
		return new GreaterEqualAction();
	}
	virtual int getPrecedence() override {
		return RelationalPrecedence;
	}
};


class EqualToken : public OperatorToken
{
public:
	EqualToken() : OperatorToken("=") {}
	virtual Action* getAction() override {
		return new EqualAction();
	}
	virtual int getPrecedence() override {
		return EqualPrecedence;
	}
};


class NotEqualToken : public OperatorToken
{
public:
	NotEqualToken() : OperatorToken("!=") {}
	virtual Action* getAction() override {
		return new NotEqualAction();
	}
	virtual int getPrecedence() override {
		return EqualPrecedence;
	}
};


class AndToken : public OperatorToken
{
public:
	AndToken() : OperatorToken("&") {}
	virtual Action* getAction() override {
		return new AndAction();
	}
	virtual int getPrecedence() override {
		return AndPrecedence;
	}
};


class OrToken : public OperatorToken
{
public:
	OrToken() : OperatorToken("|") {}
	virtual Action* getAction() override {
		return new OrAction();
	}
	virtual int getPrecedence() override {
		return OrPrecedence;
	}
};


class NotToken : public OperatorToken
{
public:
	NotToken() : OperatorToken("!") {}
	virtual Action* getAction() override {
		return new NotAction();
	}
	virtual int getPrecedence() override {
		return NotPrecedence;
	}
};


class NumberToken : public Token
{
public:
	NumberToken(string value) : Token(value) {}
	void eval(Parser& parser) override;
};

class CommaToken : public Token
{
public:
	CommaToken() : Token(",") {}
	virtual void eval(Parser& parser) override;
};

class OpenBracketToken : public Token
{
public:
	OpenBracketToken() : Token("(") {}
	void eval(Parser& parser) override;
};

class IdentifierToken : public Token
{
public:
	IdentifierToken(string value) : Token(value) {}
	void eval(Parser& parser) override;
};


class CloseBracketToken : public Token
{
public:
	CloseBracketToken() : Token(")") {}
	void eval(Parser& parser) override;
};
