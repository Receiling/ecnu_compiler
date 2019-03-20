#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <stack>
#include <cstdio>
#include <cstdlib>

using namespace std;

#define nReservedWord 30
#define maxTableLength 100
#define maxNumberLength 9
#define maxIdentLength 15
#define maxWordLength 15
#define maxError 30
#define maxAddress 2048
#define maxArrayLength 1000
#define maxVMCodeLength 200
#define stackSize 500

#define nSymbol 50
#define nObject 2
#define nFct 7
#define nType 3

enum symbol
{
	nul, identsym, numbersym, letter, plussym, minussym, times, slash,
	lparen, rparen, lbracket, rbracket, lbraces, rbraces, becomes,
	modsym, comma, semicolon, lss, gtr, orsym, andsym, notsym, xorsym,
	oddsym, eql, neq, leq, geq, ifsym, elsesym, whilesym, forsym, dosym,
	writesym, readsym, constsym, mainsym, intsym, charsym, boolsym, repeatsym,
	untilsym, exitsym, breaksym, continuesym, selfadd, selfminus, truesym, falsesym,
};

enum object
{
	constant,
	variable,
};

enum fct
{
	lit, opr, lod, sto, ini, jmp, jpc,
};

enum type
{
	inttype, chartype, booltype,
};

struct instruction
{
	enum fct f;
	int addr;
};

struct tablestruct
{
	string name;
	enum object kind;
	enum type tp;
	int val;
	int addr;
	int size;
};

class X0
{
public:
	X0();
	virtual ~X0();
	bool init(const string& codeString);
	void error(const string& message);
	bool getSym();
	bool getNextSym();
	bool program();
	bool declaration();
	bool statement(int lev);
	bool expression();
	bool additive_expr();
	bool factor();
	bool term();
	bool interpret(string& output, string& message);
	bool next(int& pc, string& output, int& top, int& inputIdx, string& message);
	void gen(const enum fct& f, const int& addr);
	int positon(const string& ident);
	bool enter(const enum object& kind, const enum type& tp, int& addressIdx);
	string getCodes(const int pc);
	string getResult();
	string getSymTable();
	string getStackStat(const int& top);
	void setInput(const string& inputString);
	
private:
	string rawCode; /*源代码*/
	string result; /*编译结果*/
	string vmCode; /*虚拟机代码*/
	string symTable; /*符号表*/
	int line;
	string ident;
	enum symbol sym;
	enum symbol nextSym;
	enum type resultType;
	int symTbx;
	int num;
	int curentPos = 0;
	int rawCodeLength;
	vector<instruction> code;
	string reservedWord[nReservedWord];
	enum symbol wsym[nReservedWord];
	enum symbol ssym[256];
	string mnemonic[nFct];
	vector<tablestruct> table;
	stack<pair<int, int>> exitStack, breakStack, continueStack;
	int stack[stackSize];
	vector<string> input;
};

