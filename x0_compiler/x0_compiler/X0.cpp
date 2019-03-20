#include "stdafx.h"
#include "X0.h"


X0::X0()
{
}

X0::~X0()
{
}

bool X0::init(const string& codeString)
{
	// 将源代码的注释内容去掉，并检查注释格式是否正确
	result = "";
	rawCodeLength = codeString.length();
	curentPos = 0;
	int commentCnt = 0;
	while (curentPos < rawCodeLength)
	{
		if (curentPos + 1 < rawCodeLength)
		{
			if (codeString[curentPos] == '/' && codeString[curentPos + 1] == '*')
			{
				commentCnt += 1;
			}
			if (codeString[curentPos] == '*' && codeString[curentPos + 1] == '/')
			{
				commentCnt -= 1;
				curentPos += 2;
				continue;
			}
		}
		// commentCnt > 0表示处在注释中，所以不加入rawCode的中
		if (commentCnt < 0)
		{
			error("注释格式错误，多余的'*/'");
			return false;
		}
		if (commentCnt == 0)
		{
			result += codeString[curentPos];
		}
		curentPos += 1;
	}
	if (commentCnt > 0)
	{
		error("注释格式错误，多余的'/*'");
		return false;
	}
	rawCode = result;
	rawCodeLength = rawCode.length();
	curentPos = 0;
	result = "0: ";
	if (rawCodeLength == 0)
	{
		error("源代码为空");
		return false;
	}

	// 初始化，清空符号表，虚拟机代码，以及exit,break,continue栈
	line = 0;
	table.clear();
	code.clear();
	while (!exitStack.empty()) exitStack.pop();
	while (!breakStack.empty()) breakStack.pop();
	while (!continueStack.empty()) continueStack.pop();

	/* 关键字的初始化 */
	for (int i = 0; i < 256; ++i)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plussym;
	ssym['-'] = minussym;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['['] = lbracket;
	ssym[']'] = rbracket;
	ssym['{'] = lbraces;
	ssym['}'] = rbraces;
	ssym['='] = becomes;
	ssym[','] = comma;
	ssym[';'] = semicolon;
	ssym['%'] = modsym;
	ssym['<'] = lss;
	ssym['>'] = gtr;
	
	reservedWord[0] = "!=";
	reservedWord[1] = "++";
	reservedWord[2] = "--";
	reservedWord[3] = "<=";
	reservedWord[4] = "==";
	reservedWord[5] = ">=";
	reservedWord[6] = "and";
	reservedWord[7] = "bool";
	reservedWord[8] = "break";
	reservedWord[9] = "char";
	reservedWord[10] = "const";
	reservedWord[11] = "continue";
	reservedWord[12] = "do";
	reservedWord[13] = "else";
	reservedWord[14] = "exit";
	reservedWord[15] = "false";
	reservedWord[16] = "for";
	reservedWord[17] = "if";
	reservedWord[18] = "int";
	reservedWord[19] = "main";
	reservedWord[20] = "not";
	reservedWord[21] = "odd";
	reservedWord[22] = "or";
	reservedWord[23] = "read";
	reservedWord[24] = "repeat";
	reservedWord[25] = "true";
	reservedWord[26] = "until";
	reservedWord[27] = "while";
	reservedWord[28] = "write";
	reservedWord[29] = "xor";

	wsym[0] = neq;
	wsym[1] = selfadd;
	wsym[2] = selfminus;
	wsym[3] = leq;
	wsym[4] = eql;
	wsym[5] = geq;
	wsym[6] = andsym;
	wsym[7] = boolsym;
	wsym[8] = breaksym;
	wsym[9] = charsym;
	wsym[10] = constsym;
	wsym[11] = continuesym;
	wsym[12] = dosym;
	wsym[13] = elsesym;
	wsym[14] = exitsym;
	wsym[15] = falsesym;
	wsym[16] = forsym;
	wsym[17] = ifsym;
	wsym[18] = intsym;
	wsym[19] = mainsym;
	wsym[20] = notsym;
	wsym[21] = oddsym;
	wsym[22] = orsym;
	wsym[23] = readsym;
	wsym[24] = repeatsym;
	wsym[25] = truesym;
	wsym[26] = untilsym;
	wsym[27] = whilesym;
	wsym[28] = writesym;
	wsym[29] = xorsym;

	mnemonic[lit] = "lit"; // 将常数值置于栈顶
	mnemonic[opr] = "opr"; // 各种运算操作
	mnemonic[lod] = "lod"; // 将变量置于栈顶
	mnemonic[sto] = "sto"; // 存储变量
	mnemonic[ini] = "ini"; // 初始化
	mnemonic[jmp] = "jmp"; // 无条件跳转
	mnemonic[jpc] = "jpc"; // 条件跳转

	return true;
}

void X0::error(const string& message)
{
	result += (" ^" + message);
}

bool X0::getSym()
{
	// 两个符号之间都使用空格，换行，制表符分隔开来，以此来提取出一个符号
	if (curentPos == rawCodeLength)
	{
		error("源代码不完整");
		return false;
	}
	char ch = rawCode[curentPos];
	while (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t')
	{
		result += ch;
		if (ch == '\n' && curentPos + 1 < rawCodeLength)
		{
			line += 1;
			result += (to_string(line) + ": ");
		}
		curentPos += 1;
		if (curentPos == rawCodeLength)
		{
			error("源代码不完整");
			return false;
		}
		ch = rawCode[curentPos];
	}
	
	ident = "";
	int length = 0;
	while (ch != ' ' && ch != '\r' && ch != '\n' && ch != '\t')
	{
		ident += ch;
		result += ch;
		length += 1;
		if (length > maxWordLength)
		{
			error("符号长度超过限制");
			return false;
		}
		curentPos += 1;
		if (curentPos == rawCodeLength) break;
		ch = rawCode[curentPos];
	}

	if ((ident[0] >= 'a' && ident[0] <= 'z') || (ident[0] >= 'A' && ident[0] <= 'Z'))
	{
		if (ident.length() > maxIdentLength)
		{
			error("用户自定义标识符长度超过限制");
			return false;
		}
		int pos = lower_bound(reservedWord, reservedWord + nReservedWord, ident) - reservedWord;
		if (pos < nReservedWord && reservedWord[pos] == ident)
		{
			sym = wsym[pos];
		}
		else
		{
			sym = identsym;
		}
	}
	else if (ident[0] >= '0' && ident[0] <= '9')
	{
		int length = ident.length();
		if (length > maxNumberLength)
		{
			error("数字长度超过限制");
			return false;
		}
		num = 0;
		for (int i = 0; i < length; ++i)
		{
			if (ident[i] < '0' || ident[i] > '9')
			{
				error("数字中包含非法字符");
				return false;
			}
			num = num * 10 + (ident[i] - '0');
		}
		sym = numbersym;
	}
	else if (ident.length() == 1)
	{
		sym = ssym[ident[0]];
		if (sym == nul)
		{
			error("非法字符,无法识别");
			return false;
		}
	}
	else if (ident[0] == '\'')
	{
		if (ident.length() == 3 && ident[2] == '\'')
		{
			num = int(ident[1]);
			sym = letter;
		}
		else
		{
			error("不是单字符");
			return false;
		}
	}
	else
	{
		// 识别组合字符，比如>= , <= ...
		int pos = lower_bound(reservedWord, reservedWord + nReservedWord, ident) - reservedWord;
		if (pos < nReservedWord && reservedWord[pos] == ident)
		{
			sym = wsym[pos];
		}
		else
		{
			error("无法识别的标识符");
			return false;
		}
	}
	return true;
}

// 用来探索后一个symbol, 而不改变curPos等状态，后来弃用
bool X0::getNextSym()
{
	// 两个符号之间都使用空格，换行，制表符分隔开来，以此来提取出一个符号
	int tmpCurcentPos = curentPos;
	if (tmpCurcentPos == rawCodeLength)
	{
		return false;
	}
	char ch = rawCode[tmpCurcentPos];
	while (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
	{
		tmpCurcentPos += 1;
		if (tmpCurcentPos == rawCodeLength)
		{
			return false;
		}
		ch = rawCode[tmpCurcentPos];
	}

	string tmpIdent = "";
	int length = 0;
	while (ch != ' ' && ch != '\n' && ch != '\t' && ch != '\r')
	{
		tmpIdent += ch;
		length += 1;
		if (length > maxWordLength)
		{
			return false;
		}
		tmpCurcentPos += 1;
		if (tmpCurcentPos == rawCodeLength) break;
		ch = rawCode[tmpCurcentPos];
	}

	if ((tmpIdent[0] >= 'a' && tmpIdent[0] <= 'z') || (tmpIdent[0] >= 'A' && tmpIdent[0] <= 'Z'))
	{
		if (tmpIdent.length() > maxIdentLength)
		{
			return false;
		}
		int pos = lower_bound(reservedWord, reservedWord + nReservedWord, tmpIdent) - reservedWord;
		if (reservedWord[pos] == tmpIdent)
		{
			nextSym = wsym[pos];
		}
		else
		{
			nextSym = identsym;
		}
	}
	else if (tmpIdent[0] >= '0' && tmpIdent[0] <= '9')
	{
		int length = tmpIdent.length();
		if (length > maxNumberLength)
		{
			return false;
		}
		for (int i = 0; i < length; ++i)
		{
			if (tmpIdent[i] < '0' || tmpIdent[i] > '9')
			{
				return false;
			}
		}
		nextSym = numbersym;
	}
	else if (tmpIdent.length() == 1)
	{
		nextSym = ssym[tmpIdent[0]];
		if (nextSym == nul)
		{
			return false;
		}
	}
	else if (tmpIdent[0] == '\'')
	{
		if (tmpIdent.length() == 3 && tmpIdent[2] == '\'')
		{
			nextSym = letter;
		}
		else
		{
			return false;
		}
	}
	else
	{
		int pos = lower_bound(reservedWord, reservedWord + nReservedWord, tmpIdent) - reservedWord;
		if (pos < nReservedWord && reservedWord[pos] == tmpIdent)
		{
			nextSym = wsym[pos];
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool X0::program()
{
	if (!getSym())
	{
		return false;
	}
	if (sym != mainsym)
	{
		error("缺少main");
		return false;
	}
	if (!getSym())
	{
		return false;
	}
	if (sym != lbraces)
	{
		error("缺少'{'");
		return false;
	}
	if (!getSym())
	{
		return false;
	}
	if (!declaration())
	{
		return false;
	}
	while (true)
	{
		if (!statement(0))
		{
			return false;
		}
		if (sym == rbraces)
		{
			break;
		}
	}
	while (!exitStack.empty())
	{
		code[exitStack.top().first].addr = code.size();
		exitStack.pop();
	}
	gen(opr, 0); // 清空数据栈
	result += "\r\n==========编译成功==============\r\n";
	return true;
}

bool X0::declaration()
{
	int addressIdx = 0;
	while (sym == constsym || sym == intsym || sym == charsym || sym == boolsym)
	{
		if (sym == constsym)
		{
			if (!getSym())
			{
				return false;
			}
			if (sym == intsym)
			{
				if (!enter(constant, inttype, addressIdx))
				{
					return false;
				}
			}
			else if (sym == charsym)
			{
				if (!enter(constant, chartype, addressIdx))
				{
					return false;
				}
			}
			else if (sym == boolsym)
			{
				if (!enter(constant, booltype, addressIdx))
				{
					return false;
				}
			}
			else
			{
				error("不支持的数据类型:" + ident);
				return false;
			}
		}
		else
		{
			if (sym == intsym)
			{
				if (!enter(variable, inttype, addressIdx))
				{
					return false;
				}
			}
			else if (sym == charsym)
			{
				if (!enter(variable, chartype, addressIdx))
				{
					return false;
				}
			}
			else if (sym == boolsym)
			{
				if (!enter(variable, booltype, addressIdx))
				{
					return false;
				}
			}
		}
	}
	gen(ini, addressIdx - 1);
	return true;
}

int X0::positon(const string& ident)
{
	for (int i = 0; i < table.size(); ++i)
	{
		if (table[i].name == ident)
		{
			return i;
		}
	}
	return -1;
}

bool X0::enter(const object& kind, const type& tp, int& addressIdx)
{
	tablestruct tb;
	tb.kind = kind;
	tb.tp = tp;
	if (kind == constant)
	{
		if (!getSym())
		{
			return false;
		}
		if (sym != identsym)
		{
			error("常量声明缺少常量名");
			return false;
		}
		int tbx = positon(ident);
		if (tbx != -1)
		{
			error("变量名重复使用");
			return false;
		}
		tb.name = ident;
		if (!getSym())
		{
			return false;
		}
		if (sym != becomes)
		{
			error("常量声明缺少赋值号");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (sym == numbersym)
		{
			if (tp != inttype)
			{
				error("数据类型不匹配");
				return false;
			}
			tb.val = num;
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("缺少分号");
				return false;
			}
			table.push_back(tb);
		}
		else if (sym == letter)
		{
			if (tp != chartype)
			{
				error("数据类型不匹配");
				return false;
			}
			tb.val = num;
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("缺少分号");
				return false;
			}
			table.push_back(tb);
		}
		else if (sym == truesym || sym == falsesym)
		{
			if (tp != booltype)
			{
				error("数据类型不匹配");
				return false;
			}
			tb.val = sym == truesym ? 1 : 0;
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("缺少分号");
				return false;
			}
			table.push_back(tb);
		}
		else
		{
			error("缺少数值，无法赋值");
			return false;
		}
	}
	else if (kind == variable)
	{
		if (!getSym())
		{
			return false;
		}
		if (sym != identsym)
		{
			error("变量声明缺少变量名");
			return false;
		}
		int tbx = positon(ident);
		if (tbx != -1)
		{
			error("变量名重复使用");
			return false;
		}
		tb.name = ident;
		tb.addr = addressIdx;
		if (!getSym())
		{
			return false;
		}
		if (sym == semicolon)
		{
			addressIdx += 1;
			if (addressIdx > maxAddress)
			{
				error("地址越界");
				return false;
			}
			tb.size = 1;
			table.push_back(tb);
		}
		else if (sym == lbracket)
		{
			if (!getSym())
			{
				return false;
			}
			if (sym != numbersym)
			{
				error("数组大小未指定");
				return false;
			}
			if (num > maxArrayLength)
			{
				error("数组空间过大");
				return false;
			}
			addressIdx += num;
			if (addressIdx > maxAddress)
			{
				error("地址越界");
				return false;
			}
			tb.size = num;
			if (!getSym())
			{
				return false;
			}
			if (sym != rbracket)
			{
				error("数组声明缺少']'");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("缺少分号");
				return false;
			}
			table.push_back(tb);
		}
		else
		{
			error("缺少分号");
			return false;
		}
	}
	else
	{
		return false;
	}
	return getSym();
}

bool X0::statement(int lev)
{
	if (sym == ifsym)
	{
		if (!getSym())
		{
			return false;
		}
		if (sym != lparen)
		{ 
			error("缺少'('");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (!expression())
		{
			return false;
		}
		if (sym != rparen)
		{
			error("缺少')'");
			return false;
		}
		gen(jpc, code.size() + 2);
		int addressIdx = code.size();
		gen(jmp, 0);
		if (!getSym())
		{
			return false;
		}
		if (!statement(lev))
		{
			return false;
		}
		code[addressIdx].addr = code.size();
		if (sym == elsesym)
		{
			if (!getSym())
			{
				return false;
			}
			if (!statement(lev))
			{
				return false;
			}
			return true;
		}
		return true;
	}
	else if (sym == whilesym)
	{
		if (!getSym())
		{
			return false;
		}
		if (sym != lparen)
		{
			error("缺少'('");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		int startAddressIdx = code.size();
		if (!expression())
		{
			return false;
		}
		if (sym != rparen)
		{
			error("缺少')'");
			return false;
		}
		gen(jpc, code.size() + 2);
		int jumpAddressIdx = code.size();
		gen(jmp, 0);
		if (!getSym())
		{
			return false;
		}
		if (!statement(lev + 1))
		{
			return false;
		}
		gen(jmp, startAddressIdx);
		code[jumpAddressIdx].addr = code.size();
		while (!continueStack.empty() && continueStack.top().second == lev + 1)
		{
			code[continueStack.top().first].addr = startAddressIdx;
			continueStack.pop();
		}
		while (!breakStack.empty() && breakStack.top().second == lev + 1)
		{
			code[breakStack.top().first].addr = code.size();
			breakStack.pop();
		}
		return true;
	}
	else if (sym == dosym)
	{
		if (!getSym())
		{
			return false;
		}
		int startAddressIdx = code.size();
		if (!statement(lev + 1))
		{
			return false;
		}
		int jumpAddressIdx = code.size();
		if (sym != whilesym)
		{
			error("缺少'while'");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != lparen)
		{
			error("缺少'('");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (!expression())
		{
			return false;
		}
		if (sym != rparen)
		{
			error("缺少')'");
			return false;
		}
		gen(jpc, startAddressIdx);
		while (!continueStack.empty() && continueStack.top().second == lev + 1)
		{
			code[continueStack.top().first].addr = jumpAddressIdx;
			continueStack.pop();
		}
		while (!breakStack.empty() && breakStack.top().second == lev + 1)
		{
			code[breakStack.top().first].addr = code.size();
			breakStack.pop();
		}
	}
	else if (sym == repeatsym)
	{
		if (!getSym())
		{
			return false;
		}
		int startAddressIdx = code.size();
		if (!statement(lev + 1))
		{
			return false;
		}
		int jumpAddressIdx = code.size();
		if (sym != untilsym)
		{
			error("缺少'until'");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != lparen)
		{
			error("缺少'('");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (!expression())
		{
			return false;
		}
		if (sym != rparen)
		{
			error("缺少')'");
			return false;
		}
		gen(jpc, code.size() + 2);
		gen(jmp, startAddressIdx);
		while (!continueStack.empty() && continueStack.top().second == lev + 1)
		{
			code[continueStack.top().first].addr = jumpAddressIdx;
			continueStack.pop();
		}
		while (!breakStack.empty() && breakStack.top().second == lev + 1)
		{
			code[breakStack.top().first].addr = code.size();
			breakStack.pop();
		}
	}
	else if (sym == forsym)
	{
		if (!getSym())
		{
			return false;
		}
		if (sym != lparen)
		{
			error("缺少'('");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (!statement(lev))
		{
			return false;
		}
		int startAddressIdx = code.size();
		if (!expression())
		{
			return false;
		}
		if (sym != semicolon)
		{
			error("缺少';'");
			return false;
		}
		int backAddressIdx = code.size();
		gen(jpc, 0);
		gen(jmp, 0);
		int jumpAddressIdx = code.size();
		if (!getSym())
		{
			return false;
		}
		if (!statement(lev + 1))
		{
			return false;
		}
		if (sym != rparen)
		{
			error("缺少')'");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		gen(jmp, startAddressIdx);
		code[backAddressIdx].addr = code.size();
		if (!statement(lev + 1))
		{
			return false;
		}
		gen(jmp, jumpAddressIdx);
		code[backAddressIdx + 1].addr = code.size();
		while (!continueStack.empty() && continueStack.top().second == lev + 1)
		{
			code[continueStack.top().first].addr = jumpAddressIdx;
			continueStack.pop();
		}
		while (!breakStack.empty() && breakStack.top().second == lev + 1)
		{
			code[breakStack.top().first].addr = code.size();
			breakStack.pop();
		}
		return true;
	}
	else if (sym == readsym)
	{
		if (!getSym())
		{
			return false;
		}
		if (sym != identsym)
		{
			error("缺少要读入的变量名");
			return false;
		}
		int tbx = positon(ident);
		if (tbx == -1)
		{
			error("变量未声明");
			return false;
		}
		if (table[tbx].kind == constant)
		{
			error("不可修改常量的值");
			return false;
		}
		if (table[tbx].size == 1)
		{
			if (table[tbx].tp == inttype)
			{
				gen(opr, 1); // 将读入的int置于栈顶
			}
			else if (table[tbx].tp == chartype)
			{
				gen(opr, 21); // 将读入的char置于栈顶
			}
			else if (table[tbx].tp == booltype)
			{
				gen(opr, 22); // 将读入的bool置于栈顶
			}
			gen(sto, table[tbx].addr);
		}
		else
		{
			if (!getSym())
			{
				return false;
			}
			if (sym != lbracket)
			{
				error("缺少数组索引");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!expression())
			{
				return false;
			}
			gen(lit, table[tbx].addr);
			gen(opr, 2); // s[t - 1] = s[t] + s[t - 1]
			if (table[tbx].tp == inttype)
			{
				gen(opr, 1); // 将读入的int置于栈顶
			}
			else if (table[tbx].tp == chartype)
			{
				gen(opr, 21); // 将读入的char置于栈顶
			}
			else if (table[tbx].tp == booltype)
			{
				gen(opr, 22); // 将读入的bool置于栈顶
			}
			gen(sto, -1); // 将栈顶的值存到次栈顶作为index的位置
			if (!getSym())
			{
				return false;
			}
			if (sym != rbracket)
			{
				error("缺少']'");
				return false;
			}
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != semicolon)
		{
			error("缺少分号");
			return false;
		}
	}
	else if (sym == writesym)
	{
		if (!getSym())
		{
			return false;
		}
		if (!expression())
		{
			return false;
		}
		if (resultType == inttype)
		{
			gen(opr, 3); // 生成输出int指令，输出栈顶的值
		}
		else if (resultType == chartype)
		{
			gen(opr, 23); // 生成输出char指令，输出栈顶的值
		}
		else if (resultType == booltype)
		{
			gen(opr, 24); // 生成输出bool指令，输出栈顶的值
		}
		gen(opr, 4); //	生成换行指令
		if (sym != semicolon)
		{
			error("缺少分号");
			return false;
		}
	}
	else if (sym == lbraces)
	{
		if (!getSym())
		{
			return false;
		}
		while (sym == ifsym || sym == whilesym || sym == dosym ||
			sym == repeatsym || sym == forsym || sym == readsym ||
			sym == writesym || sym == lbraces || sym == exitsym ||
			sym == continuesym || sym == breaksym || sym == identsym)
		{
			if (!statement(lev))
			{
				return false;
			}
		}
		if (sym != rbraces)
		{
			error("缺少'}'");
			return false;
		}
	}
	else if (sym == exitsym)
	{
		exitStack.push(make_pair(code.size(), lev));
		gen(jmp, 0);
		if (!getSym())
		{
			return false;
		}
		if (sym != semicolon)
		{
			error("缺少分号");
			return false;
		}
	}
	else if (sym == continuesym)
	{
		continueStack.push(make_pair(code.size(), lev));
		gen(jmp, 0);
		if (!getSym())
		{
			return false;
		}
		if (sym != semicolon)
		{
			error("缺少分号");
			return false;
		}
	}
	else if (sym == breaksym)
	{
		breakStack.push(make_pair(code.size(), lev));
		gen(jmp, 0);
		if (!getSym())
		{
			return false;
		}
		if (sym != semicolon)
		{
			error("缺少分号");
			return false;
		}
	}
	else if (sym == identsym)
	{
		int tbx = positon(ident);
		if (tbx == -1)
		{
			error("变量未声明");
			return false;
		}
		if (table[tbx].kind == constant)
		{
			error("不可修改常量的值");
			return false;
		}
		if (table[tbx].size > 1)
		{
			if (!getSym())
			{
				return false;
			}
			if (sym != lbracket)
			{
				error("缺少数组索引");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!expression())
			{
				return false;
			}
			gen(lit, table[tbx].addr);
			gen(opr, 2); // s[t - 1] = s[t] + s[t - 1]
			if (sym != rbracket)
			{
				error("缺少']'");
				return false;
			}
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != becomes)
		{
			error("缺少赋值号");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (!expression())
		{
			return false;
		}
		if (table[tbx].size == 1)
		{
			gen(sto, table[tbx].addr);
		}
		else
		{
			gen(sto, -1);
		}
		if (sym != semicolon)
		{
			error("缺少分号");
			return false;
		}
	}
	else if (sym == rbraces)
	{
		return true;
	}
	else if (sym != semicolon)
	{
		return false;
	}
	return getSym();
}

bool X0::expression()
{
	if (sym == oddsym)
	{
		if (!getSym())
		{
			return false;
		}
		if (!additive_expr())
		{
			return false;
		}
		if (resultType != inttype)
		{
			error("不可对非int类型执行判断奇偶运算");
			return false;
		}
		resultType = booltype;
		gen(opr, 5); // 判断奇偶
	}
	else
	{
		if (!additive_expr())
		{
			return false;
		}
		// 支持int和char比较大小
		if (sym == eql)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!additive_expr())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			resultType = booltype;
			gen(opr, 6); // 判断相等
		}
		else if (sym == neq)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!additive_expr())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			resultType = booltype;
			gen(opr, 7); // 判断不等
		}
		else if (sym == lss)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!additive_expr())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			resultType = booltype;
			gen(opr, 8); // 判断小于
		}
		else if (sym == geq)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!additive_expr())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			resultType = booltype;
			gen(opr, 9); // 判断大于等于
		}
		else if (sym == gtr)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!additive_expr())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			resultType = booltype;
			gen(opr, 10); // 判断大于
		}
		else if (sym == leq)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (!additive_expr())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型执行比较运算");
				return false;
			}
			resultType = booltype;
			gen(opr, 11); // 判断小于等于
		}
	}
	return true;
}

bool X0::additive_expr()
{
	if (!term())
	{
		return false;
	}
	while (sym == plussym || sym == minussym || sym == orsym)
	{
		if (sym == plussym)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型加法运算");
				return false;
			}
			enum type lastType = resultType;
			if (!getSym())
			{
				return false;
			}
			if (!term())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型加法运算");
				return false;
			}
			if (lastType == chartype || resultType == chartype)
			{
				resultType = chartype;
			}
			else
			{
				resultType = inttype;
			}
			gen(opr, 2); // s[t - 1] = s[t] + s[t - 1]
		}
		else if (sym == minussym)
		{
			if (resultType == booltype)
			{
				error("不可对bool类型减法运算");
				return false;
			}
			enum type lastType = resultType;
			if (!getSym())
			{
				return false;
			}
			if (!term())
			{
				return false;
			}
			if (resultType == booltype)
			{
				error("不可对bool类型减法运算");
				return false;
			}
			if (lastType == chartype || resultType == chartype)
			{
				resultType = chartype;
			}
			else
			{
				resultType = chartype;
			}
			gen(opr, 12); // s[t - 1] = s[t - 1] - s[t]
		}
		else if (sym == orsym)
		{
			if (!getSym())
			{
				return false;
			}
			if (!term())
			{
				return false;
			}
			resultType = booltype;
			gen(opr, 13); // s[t - 1] = s[t - 1] or s[t]
		}
	}
	return true;
}

bool X0::term()
{
	if (!factor())
	{
		return false;
	}
	if (sym == selfadd)
	{
		if (resultType != inttype)
		{
			error("不可对非int类型执行自增运算");
			return false;
		}
		resultType = inttype;
		gen(opr, 18); // s[t] = s[t] + 1
		gen(sto, table[symTbx].addr);
		return getSym();
	}
	else if (sym == selfminus)
	{
		if (resultType != inttype)
		{
			error("不可对非int类型执行自减运算");
			return false;
		}
		resultType = inttype;
		gen(opr, 19); // s[t] = s[t] - 1
		gen(sto, table[symTbx].addr);
		return getSym();
	}
	else
	{
		while (sym == times || sym == slash || sym == andsym || sym == modsym || sym == xorsym)
		{
			if (sym == times)
			{
				if (resultType != inttype)
				{
					error("不可对非int类型执行乘法运算");
					return false;
				}
				if (!getSym())
				{
					return false;
				}
				if (!factor())
				{
					return false;
				}
				if (resultType != inttype)
				{
					error("不可对非int类型执行乘法运算");
					return false;
				}
				resultType = inttype;
				gen(opr, 14); // s[t - 1] = s[t - 1] * s[t]
			}
			else if (sym == slash)
			{
				if (resultType != inttype)
				{
					error("不可对非int类型执行除法运算");
					return false;
				}
				if (!getSym())
				{
					return false;
				}
				if (!factor())
				{
					return false;
				}
				if (resultType != inttype)
				{
					error("不可对非int类型执行除法运算");
					return false;
				}
				resultType = inttype;
				gen(opr, 15); // s[t - 1] = s[t - 1] / s[t] 
			}
			else if (sym == andsym)
			{
				if (!getSym())
				{
					return false;
				}
				if (!factor())
				{
					return false;
				}
				resultType = booltype;
				gen(opr, 25); // s[t - 1] = s[t - 1] and s[t]
			}
			else if (sym == modsym)
			{
				if (resultType != inttype)
				{
					error("不可对非int类型执行取余运算");
					return false;
				}
				if (!getSym())
				{
					return false;
				}
				if (!factor())
				{
					return false;
				}
				if (resultType != inttype)
				{
					error("不可对非int类型执行取余运算");
					return false;
				}
				resultType = inttype;
				gen(opr, 16); // s[t - 1] = s[t - 1] % s[t]
			}
			else if (sym == xorsym)
			{
				if (resultType != inttype)
				{
					error("不可对非int类型执行异或运算");
					return false;
				}
				if (!getSym())
				{
					return false;
				}
				if (!factor())
				{
					return false;
				}
				if (resultType != inttype)
				{
					error("不可对非int类型执行异或运算");
					return false;
				}
				resultType = inttype;
				gen(opr, 17); // s[t - 1] = s[t - 1] xor s[t]
			}
		}
	}
	return true;
}

bool X0::factor()
{
	if (sym == lparen)
	{
		if (!getSym())
		{
			return false;
		}
		if (!expression())
		{
			return false;
		}
		if (sym != rparen)
		{
			error("缺少')'");
			return false;
		}
	}
	else if (sym == identsym)
	{
		int tbx = positon(ident);
		symTbx = tbx;
		if (tbx == -1)
		{
			error("变量未声明");
			return false;
		}
		if (table[tbx].kind == constant)
		{
			gen(lit, table[tbx].val);
		}
		else
		{
			if (table[tbx].size == 1)
			{
				gen(lod, table[tbx].addr);
			}
			else
			{
				if (!getSym())
				{
					return false;
				}
				if (sym != lbracket)
				{
					error("缺少数组索引");
					return false;
				}
				if (!getSym())
				{
					return false;
				}
				if (!expression())
				{
					return false;
				}
				gen(lit, table[tbx].addr);
				gen(opr, 2);
				gen(lod, -1); // 间接取值，将栈顶的值作为index去寻址，将对应位置的值放到到栈顶
				if (sym != rbracket)
				{
					error("缺少']'");
					return false;
				}
			}
		}
		resultType = table[tbx].tp;
	}
	else if (sym == numbersym)
	{
		resultType = inttype;
		gen(lit, num);
	}
	else if (sym == letter)
	{
		resultType = chartype;
		gen(lit, num);
	}
	else if (sym == truesym)
	{
		resultType = booltype;
		gen(lit, 1);
	}
	else if (sym == falsesym)
	{
		resultType = booltype;
		gen(lit, 0);
	}
	else if (sym == notsym)
	{
		if (!getSym())
		{
			return false;
		}
		if (!factor())
		{
			return false;
		}
		// 此处取反把所有非0变为0，把0变为1，适用于三种数据类型
		resultType = booltype;
		gen(opr, 20); // s[t] = not s[t]
		return true;
	}
	return getSym();
}

void X0::gen(const fct& f, const int& addr)
{
	instruction inst;
	inst.f = f;
	inst.addr = addr;
	code.push_back(inst);
}

bool X0::interpret(string& output, string& message)
{
	int pc = 0;
	int top = -1;
	int inputIdx = 0;
	output = "===============Start X0===============\r\n";
	do
	{
		if (!next(pc, output, top, inputIdx, message))
		{
			return false;
		}
	} while (pc != 0);
	output += "===============End X0================\r\n";
	return true;
}

string X0::getResult()
{
	return result;
}

string X0::getCodes(const int pc)
{
	vmCode = "===============VMCODE===============\r\nPC\t\tFct\t\tAddr\r\n";
	for (int i = 0; i < code.size(); ++i)
	{
		if (i == pc)
		{
			vmCode += ">>\t\t";
		}
		else
		{
			vmCode += (to_string(i) + "\t\t");
		}
		vmCode += (mnemonic[code[i].f] + "\t\t" + to_string(code[i].addr) + "\r\n");
	}
	return vmCode;
}

string X0::getSymTable()
{
	symTable = "===============SYMTABLE===============\r\nName\tKind\tType\tVal\tAddr\tSize\r\n";
	for (int i = 0; i < table.size(); ++i)
	{
		symTable += (table[i].name + '\t');
		if (table[i].kind == constant)
		{
			symTable += "cosnt\t";
		}
		else
		{
			symTable += "variable\t";
		}

		if (table[i].tp == inttype)
		{
			symTable += "int\t";
		}
		else if (table[i].tp == chartype)
		{
			symTable += "char\t";
		}
		else
		{
			symTable += "bool\t";
		}

		if (table[i].kind == constant)
		{
			if (table[i].tp == inttype)
			{
				symTable += to_string(table[i].val);
			}
			else if (table[i].tp == chartype)
			{
				char c = char(table[i].val);
				symTable += c;
			}
			else
			{
				string b = table[i].val == 0 ? "false" : "true";
				symTable += b;
			}
			symTable += "\t_\t_\r\n";
		}
		else
		{
			symTable += ("_\t" + to_string(table[i].addr) + "\t" + to_string(table[i].size) + "\r\n");
		}
	}
	return symTable;
}

string X0::getStackStat(const int& top)
{
	string stackStat = "=======STACK=======\r\nN\t\tVal\r\n";
	for (int i = 0; i <= top; ++i)
	{
		stackStat += (to_string(i) + "\t\t" + to_string(stack[i]) + "\r\n");
	}
	return stackStat;
}

bool X0::next(int& pc, string& output, int& top, int& inputIdx, string& message)
{
	instruction inst = code[pc];
	pc += 1;
	char c;
	string in;
	switch (inst.f)
	{
	case lit:
		top += 1;
		stack[top] = inst.addr;
		break;
	case opr:
		switch (inst.addr)
		{
		case 0:
			pc = 0;
			top = -1;
			break;
		case 1:
		{
			if (inputIdx >= input.size())
			{
				message = "缺少输入";
				return false;
			}
			int length = input[inputIdx].length();
			if (length > maxNumberLength)
			{
				message = "输入的数字溢出";
				return false;
			}
			for (int i = 0; i < length; ++i)
			{
				if (input[inputIdx][i] < '0' || input[inputIdx][i] > '9')
				{
					message = "输入的数字中包含字母";
					return false;
				}
			}
			top += 1;
			output += "?";
			stack[top] = atoi(input[inputIdx].c_str());
			output += (input[inputIdx] + "\r\n");
			inputIdx += 1;
		}
		break;
		case 2:
			top -= 1;
			stack[top] += stack[top + 1];
			break;
		case 3:
			output += (to_string(stack[top]));
			top -= 1;
			break;
		case 4:
			output += "\r\n";
			break;
		case 5:
			stack[top] = stack[top] % 2;
			break;
		case 6:
			top -= 1;
			stack[top] = (stack[top] == stack[top + 1]) ? 1 : 0;
			break;
		case 7:
			top -= 1;
			stack[top] = (stack[top] != stack[top + 1]) ? 1 : 0;
			break;
		case 8:
			top -= 1;
			stack[top] = (stack[top] < stack[top + 1]) ? 1 : 0;
			break;
		case 9:
			top -= 1;
			stack[top] = (stack[top] >= stack[top + 1]) ? 1 : 0;
			break;
		case 10:
			top -= 1;
			stack[top] = (stack[top] > stack[top + 1]) ? 1 : 0;
			break;
		case 11:
			top -= 1;
			stack[top] = (stack[top] <= stack[top + 1]) ? 1 : 0;
			break;
		case 12:
			top -= 1;
			stack[top] -= stack[top + 1];
			break;
		case 13:
			top -= 1;
			stack[top] = (stack[top] || stack[top + 1]) ? 1 : 0;
			break;
		case 14:
			top -= 1;
			stack[top] *= stack[top + 1];
			break;
		case 15:
			top -= 1;
			stack[top] = int(stack[top] / stack[top + 1]);
			break;
		case 16:
			top -= 1;
			stack[top] = stack[top] % stack[top + 1];
			break;
		case 17:
			top -= 1;
			stack[top] = stack[top] ^ stack[top + 1];
			break;
		case 18:
			stack[top] += 1;
			break;
		case 19:
			stack[top] -= 1;
			break;
		case 20:
			stack[top] = (stack[top]) ? 0 : 1;
			break;
		case 21:
		{
			if (inputIdx >= input.size())
			{
				message = "缺少输入";
				return false;
			}
			if (input[inputIdx].length() != 1)
			{
				message = "缺少不是单个字符";
				return false;
			}
			top += 1;
			output += "?";
			stack[top] = int(input[inputIdx][0]);
			output += (input[inputIdx] + "\r\n");
			inputIdx += 1;
		}
		break;
		case 22:
		{
			if (inputIdx >= input.size())
			{
				message = "缺少输入";
				return false;
			}
			if (input[inputIdx] != "true" && input[inputIdx] != "false")
			{
				message = "输入不是bool类型";
				return false;
			}
			top += 1;
			output += "?";
			stack[top] = input[inputIdx] == "true" ? 1 : 0;
			output += (input[inputIdx] + "\r\n");
			inputIdx += 1;
		}
		break;
		case 23:
			c = char(stack[top]);
			output += c;
			top -= 1;
			break;
		case 24:
			if (stack[top] == 0)
			{
				output += "false";
			}
			else
			{
				output += "true";
			}
			top -= 1;
			break;
		case 25:
			top -= 1;
			stack[top] = (stack[top] && stack[top + 1]) ? 1 : 0;
			break;
		}
		break;
	case lod:
		if (inst.addr == -1)
		{
			stack[top] = stack[stack[top]];
		}
		else
		{
			top += 1;
			stack[top] = stack[inst.addr];
		}
		break;
	case sto:
		if (inst.addr == -1)
		{
			stack[stack[top - 1]] = stack[top];
			top -= 2;
		}
		else
		{
			stack[inst.addr] = stack[top];
		}
		break;
	case jmp:
		pc = inst.addr;
		break;
	case jpc:
		if (stack[top] != 0)
		{
			pc = inst.addr;
		}
		top -= 1;
		break;
	case ini:
		top = inst.addr;
		for (int i = 0; i <= top; ++i)
		{
			stack[i] = 0;
		}
		break;
	}
	return true;
}

void X0::setInput(const string& inputString)
{
	input.clear();
	int length = inputString.length();
	int curPos = 0;
	while (curPos < length)
	{
		while (curPos < length && (inputString[curPos] == '\r' || inputString[curPos] == '\n' ||
			inputString[curPos] == '\t' || inputString[curPos] == ' '))
		{
			curPos += 1;
		}
		string str = "";
		while (curPos < length && inputString[curPos] != '\r' && inputString[curPos] != '\n' &&
			inputString[curPos] != '\t' && inputString[curPos] != ' ')
		{
			str += inputString[curPos];
			curPos += 1;
		}
		if (str != "")
		{
			input.push_back(str);
		}
	}
}