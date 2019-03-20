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
	// ��Դ�����ע������ȥ���������ע�͸�ʽ�Ƿ���ȷ
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
		// commentCnt > 0��ʾ����ע���У����Բ�����rawCode����
		if (commentCnt < 0)
		{
			error("ע�͸�ʽ���󣬶����'*/'");
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
		error("ע�͸�ʽ���󣬶����'/*'");
		return false;
	}
	rawCode = result;
	rawCodeLength = rawCode.length();
	curentPos = 0;
	result = "0: ";
	if (rawCodeLength == 0)
	{
		error("Դ����Ϊ��");
		return false;
	}

	// ��ʼ������շ��ű���������룬�Լ�exit,break,continueջ
	line = 0;
	table.clear();
	code.clear();
	while (!exitStack.empty()) exitStack.pop();
	while (!breakStack.empty()) breakStack.pop();
	while (!continueStack.empty()) continueStack.pop();

	/* �ؼ��ֵĳ�ʼ�� */
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

	mnemonic[lit] = "lit"; // ������ֵ����ջ��
	mnemonic[opr] = "opr"; // �����������
	mnemonic[lod] = "lod"; // ����������ջ��
	mnemonic[sto] = "sto"; // �洢����
	mnemonic[ini] = "ini"; // ��ʼ��
	mnemonic[jmp] = "jmp"; // ��������ת
	mnemonic[jpc] = "jpc"; // ������ת

	return true;
}

void X0::error(const string& message)
{
	result += (" ^" + message);
}

bool X0::getSym()
{
	// ��������֮�䶼ʹ�ÿո񣬻��У��Ʊ���ָ��������Դ�����ȡ��һ������
	if (curentPos == rawCodeLength)
	{
		error("Դ���벻����");
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
			error("Դ���벻����");
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
			error("���ų��ȳ�������");
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
			error("�û��Զ����ʶ�����ȳ�������");
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
			error("���ֳ��ȳ�������");
			return false;
		}
		num = 0;
		for (int i = 0; i < length; ++i)
		{
			if (ident[i] < '0' || ident[i] > '9')
			{
				error("�����а����Ƿ��ַ�");
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
			error("�Ƿ��ַ�,�޷�ʶ��");
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
			error("���ǵ��ַ�");
			return false;
		}
	}
	else
	{
		// ʶ������ַ�������>= , <= ...
		int pos = lower_bound(reservedWord, reservedWord + nReservedWord, ident) - reservedWord;
		if (pos < nReservedWord && reservedWord[pos] == ident)
		{
			sym = wsym[pos];
		}
		else
		{
			error("�޷�ʶ��ı�ʶ��");
			return false;
		}
	}
	return true;
}

// ����̽����һ��symbol, �����ı�curPos��״̬����������
bool X0::getNextSym()
{
	// ��������֮�䶼ʹ�ÿո񣬻��У��Ʊ���ָ��������Դ�����ȡ��һ������
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
		error("ȱ��main");
		return false;
	}
	if (!getSym())
	{
		return false;
	}
	if (sym != lbraces)
	{
		error("ȱ��'{'");
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
	gen(opr, 0); // �������ջ
	result += "\r\n==========����ɹ�==============\r\n";
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
				error("��֧�ֵ���������:" + ident);
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
			error("��������ȱ�ٳ�����");
			return false;
		}
		int tbx = positon(ident);
		if (tbx != -1)
		{
			error("�������ظ�ʹ��");
			return false;
		}
		tb.name = ident;
		if (!getSym())
		{
			return false;
		}
		if (sym != becomes)
		{
			error("��������ȱ�ٸ�ֵ��");
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
				error("�������Ͳ�ƥ��");
				return false;
			}
			tb.val = num;
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("ȱ�ٷֺ�");
				return false;
			}
			table.push_back(tb);
		}
		else if (sym == letter)
		{
			if (tp != chartype)
			{
				error("�������Ͳ�ƥ��");
				return false;
			}
			tb.val = num;
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("ȱ�ٷֺ�");
				return false;
			}
			table.push_back(tb);
		}
		else if (sym == truesym || sym == falsesym)
		{
			if (tp != booltype)
			{
				error("�������Ͳ�ƥ��");
				return false;
			}
			tb.val = sym == truesym ? 1 : 0;
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("ȱ�ٷֺ�");
				return false;
			}
			table.push_back(tb);
		}
		else
		{
			error("ȱ����ֵ���޷���ֵ");
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
			error("��������ȱ�ٱ�����");
			return false;
		}
		int tbx = positon(ident);
		if (tbx != -1)
		{
			error("�������ظ�ʹ��");
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
				error("��ַԽ��");
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
				error("�����Сδָ��");
				return false;
			}
			if (num > maxArrayLength)
			{
				error("����ռ����");
				return false;
			}
			addressIdx += num;
			if (addressIdx > maxAddress)
			{
				error("��ַԽ��");
				return false;
			}
			tb.size = num;
			if (!getSym())
			{
				return false;
			}
			if (sym != rbracket)
			{
				error("��������ȱ��']'");
				return false;
			}
			if (!getSym())
			{
				return false;
			}
			if (sym != semicolon)
			{
				error("ȱ�ٷֺ�");
				return false;
			}
			table.push_back(tb);
		}
		else
		{
			error("ȱ�ٷֺ�");
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
			error("ȱ��'('");
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
			error("ȱ��')'");
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
			error("ȱ��'('");
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
			error("ȱ��')'");
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
			error("ȱ��'while'");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != lparen)
		{
			error("ȱ��'('");
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
			error("ȱ��')'");
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
			error("ȱ��'until'");
			return false;
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != lparen)
		{
			error("ȱ��'('");
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
			error("ȱ��')'");
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
			error("ȱ��'('");
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
			error("ȱ��';'");
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
			error("ȱ��')'");
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
			error("ȱ��Ҫ����ı�����");
			return false;
		}
		int tbx = positon(ident);
		if (tbx == -1)
		{
			error("����δ����");
			return false;
		}
		if (table[tbx].kind == constant)
		{
			error("�����޸ĳ�����ֵ");
			return false;
		}
		if (table[tbx].size == 1)
		{
			if (table[tbx].tp == inttype)
			{
				gen(opr, 1); // �������int����ջ��
			}
			else if (table[tbx].tp == chartype)
			{
				gen(opr, 21); // �������char����ջ��
			}
			else if (table[tbx].tp == booltype)
			{
				gen(opr, 22); // �������bool����ջ��
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
				error("ȱ����������");
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
				gen(opr, 1); // �������int����ջ��
			}
			else if (table[tbx].tp == chartype)
			{
				gen(opr, 21); // �������char����ջ��
			}
			else if (table[tbx].tp == booltype)
			{
				gen(opr, 22); // �������bool����ջ��
			}
			gen(sto, -1); // ��ջ����ֵ�浽��ջ����Ϊindex��λ��
			if (!getSym())
			{
				return false;
			}
			if (sym != rbracket)
			{
				error("ȱ��']'");
				return false;
			}
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != semicolon)
		{
			error("ȱ�ٷֺ�");
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
			gen(opr, 3); // �������intָ����ջ����ֵ
		}
		else if (resultType == chartype)
		{
			gen(opr, 23); // �������charָ����ջ����ֵ
		}
		else if (resultType == booltype)
		{
			gen(opr, 24); // �������boolָ����ջ����ֵ
		}
		gen(opr, 4); //	���ɻ���ָ��
		if (sym != semicolon)
		{
			error("ȱ�ٷֺ�");
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
			error("ȱ��'}'");
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
			error("ȱ�ٷֺ�");
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
			error("ȱ�ٷֺ�");
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
			error("ȱ�ٷֺ�");
			return false;
		}
	}
	else if (sym == identsym)
	{
		int tbx = positon(ident);
		if (tbx == -1)
		{
			error("����δ����");
			return false;
		}
		if (table[tbx].kind == constant)
		{
			error("�����޸ĳ�����ֵ");
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
				error("ȱ����������");
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
				error("ȱ��']'");
				return false;
			}
		}
		if (!getSym())
		{
			return false;
		}
		if (sym != becomes)
		{
			error("ȱ�ٸ�ֵ��");
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
			error("ȱ�ٷֺ�");
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
			error("���ɶԷ�int����ִ���ж���ż����");
			return false;
		}
		resultType = booltype;
		gen(opr, 5); // �ж���ż
	}
	else
	{
		if (!additive_expr())
		{
			return false;
		}
		// ֧��int��char�Ƚϴ�С
		if (sym == eql)
		{
			if (resultType == booltype)
			{
				error("���ɶ�bool����ִ�бȽ�����");
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
				error("���ɶ�bool����ִ�бȽ�����");
				return false;
			}
			resultType = booltype;
			gen(opr, 6); // �ж����
		}
		else if (sym == neq)
		{
			if (resultType == booltype)
			{
				error("���ɶ�bool����ִ�бȽ�����");
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
				error("���ɶ�bool����ִ�бȽ�����");
				return false;
			}
			resultType = booltype;
			gen(opr, 7); // �жϲ���
		}
		else if (sym == lss)
		{
			if (resultType == booltype)
			{
				error("���ɶ�bool����ִ�бȽ�����");
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
				error("���ɶ�bool����ִ�бȽ�����");
				return false;
			}
			resultType = booltype;
			gen(opr, 8); // �ж�С��
		}
		else if (sym == geq)
		{
			if (resultType == booltype)
			{
				error("���ɶ�bool����ִ�бȽ�����");
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
				error("���ɶ�bool����ִ�бȽ�����");
				return false;
			}
			resultType = booltype;
			gen(opr, 9); // �жϴ��ڵ���
		}
		else if (sym == gtr)
		{
			if (resultType == booltype)
			{
				error("���ɶ�bool����ִ�бȽ�����");
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
				error("���ɶ�bool����ִ�бȽ�����");
				return false;
			}
			resultType = booltype;
			gen(opr, 10); // �жϴ���
		}
		else if (sym == leq)
		{
			if (resultType == booltype)
			{
				error("���ɶ�bool����ִ�бȽ�����");
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
				error("���ɶ�bool����ִ�бȽ�����");
				return false;
			}
			resultType = booltype;
			gen(opr, 11); // �ж�С�ڵ���
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
				error("���ɶ�bool���ͼӷ�����");
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
				error("���ɶ�bool���ͼӷ�����");
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
				error("���ɶ�bool���ͼ�������");
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
				error("���ɶ�bool���ͼ�������");
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
			error("���ɶԷ�int����ִ����������");
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
			error("���ɶԷ�int����ִ���Լ�����");
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
					error("���ɶԷ�int����ִ�г˷�����");
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
					error("���ɶԷ�int����ִ�г˷�����");
					return false;
				}
				resultType = inttype;
				gen(opr, 14); // s[t - 1] = s[t - 1] * s[t]
			}
			else if (sym == slash)
			{
				if (resultType != inttype)
				{
					error("���ɶԷ�int����ִ�г�������");
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
					error("���ɶԷ�int����ִ�г�������");
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
					error("���ɶԷ�int����ִ��ȡ������");
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
					error("���ɶԷ�int����ִ��ȡ������");
					return false;
				}
				resultType = inttype;
				gen(opr, 16); // s[t - 1] = s[t - 1] % s[t]
			}
			else if (sym == xorsym)
			{
				if (resultType != inttype)
				{
					error("���ɶԷ�int����ִ���������");
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
					error("���ɶԷ�int����ִ���������");
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
			error("ȱ��')'");
			return false;
		}
	}
	else if (sym == identsym)
	{
		int tbx = positon(ident);
		symTbx = tbx;
		if (tbx == -1)
		{
			error("����δ����");
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
					error("ȱ����������");
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
				gen(lod, -1); // ���ȡֵ����ջ����ֵ��ΪindexȥѰַ������Ӧλ�õ�ֵ�ŵ���ջ��
				if (sym != rbracket)
				{
					error("ȱ��']'");
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
		// �˴�ȡ�������з�0��Ϊ0����0��Ϊ1��������������������
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
				message = "ȱ������";
				return false;
			}
			int length = input[inputIdx].length();
			if (length > maxNumberLength)
			{
				message = "������������";
				return false;
			}
			for (int i = 0; i < length; ++i)
			{
				if (input[inputIdx][i] < '0' || input[inputIdx][i] > '9')
				{
					message = "����������а�����ĸ";
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
				message = "ȱ������";
				return false;
			}
			if (input[inputIdx].length() != 1)
			{
				message = "ȱ�ٲ��ǵ����ַ�";
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
				message = "ȱ������";
				return false;
			}
			if (input[inputIdx] != "true" && input[inputIdx] != "false")
			{
				message = "���벻��bool����";
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