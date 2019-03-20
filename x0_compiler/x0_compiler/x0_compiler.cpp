// x0_compiler.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "x0_compiler.h"
#include "X0.h"

#define MAX_LOADSTRING 100
#define ID_EDIT	1
#define UNTITLED TEXT("(untitled)")

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
OPENFILENAME ofn;
string inputString;

// 此代码模块中包含的函数的前向声明:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	InputDlgProc(HWND, UINT, WPARAM, LPARAM);
void PopFileInitialize(HWND);
BOOL PopFileOpenDlg(HWND, PTSTR, PTSTR);
BOOL PopFileRead(HWND, PTSTR);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR szCmdLine,
	int iCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(szCmdLine);

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_X0_COMPILER, szWindowClass, MAX_LOADSTRING);

	// 注册窗口类
	WNDCLASSEXW wcex;
	::ZeroMemory(&wcex, sizeof(wcex));
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_X0_COMPILER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_X0_COMPILER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassExW(&wcex))
	{
		MessageBox(NULL, TEXT("Failed to Register WinMainClass!"),
			szWindowClass, MB_ICONERROR);
		return 0;
	}

	// 创建主窗口
	hInst = hInstance; // 将实例句柄存储在全局变量中

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		MessageBox(NULL, TEXT("Failed to CreateWindow!"),
			szWindowClass, MB_ICONERROR);
		return 0;
	}

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_X0_COMPILER));

	MSG msg;

	// 主消息循环: 
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	DestroyAcceleratorTable(hAccelTable);
	return (int)msg.wParam;
}

// 显示文件名
void DoCaption(HWND hwnd, TCHAR *szTitleName)
{
	TCHAR szCaption[64 + MAX_PATH];
	wsprintf(szCaption, TEXT("%s - %s"), szWindowClass, szTitleName[0] ? szTitleName : UNTITLED);
	SetWindowText(hwnd, szCaption);
}

// 提示对话框
void OkMessage(HWND hwnd, TCHAR *szMessage, TCHAR *szTitleName)
{
	TCHAR szBuffer[64 + MAX_PATH];
	wsprintf(szBuffer, szMessage, szTitleName[0] ? szTitleName : UNTITLED);
	MessageBox(hwnd, szBuffer, szWindowClass, MB_OK | MB_ICONEXCLAMATION);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndEdit;
	static HWND hwndOutput;
	static HWND hwndInstruction;
	static HWND hwndStack;
	static HMENU hMenu;
	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH];
	static X0 x0 = X0();
	static int pc = 0;
	static int top = -1;
	static string output;
	static string  errorMessage;
	static int inputIdx = 0;

	switch (message)
	{
	case WM_CREATE:
	{
		hwndEdit = CreateWindowW(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL |
			WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			0, 0, 0, 0, hWnd, (HMENU)ID_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		SendMessage(hwndEdit, EM_LIMITTEXT, 32000, 0L);
		hwndOutput = CreateWindowW(TEXT("edit"), TEXT("OUTPUT:"), WS_CHILD | WS_VISIBLE | WS_HSCROLL |
			WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY,
			0, 0, 0, 0, hWnd, (HMENU)ID_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		hwndInstruction = CreateWindowW(TEXT("edit"), TEXT("INSTRUCTION:"), WS_CHILD | WS_VISIBLE | WS_HSCROLL |
			WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY,
			0, 0, 0, 0, hWnd, (HMENU)ID_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		hwndStack = CreateWindowW(TEXT("edit"), TEXT("STACK:"), WS_CHILD | WS_VISIBLE | WS_HSCROLL |
			WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY,
			0, 0, 0, 0, hWnd, (HMENU)ID_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		hMenu = GetMenu(hWnd);
		EnableMenuItem(hMenu, IDM_RUN, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_DEBUG, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_NEXT, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_TABLE, MF_GRAYED);
		PopFileInitialize(hWnd);
		DoCaption(hWnd, szTitleName);
	}
	break;
	case WM_ACTIVATE: // 当窗口被激活时，将焦点设置在文本框上
	{
		//SetFocus(hwndEdit);
	}
	break;
	case WM_SIZE:
	{
		MoveWindow(hwndEdit, 0, 0, (int)(LOWORD(lParam) * 0.7), (int)(HIWORD(lParam) * 0.7), TRUE);
		MoveWindow(hwndOutput, 0, (int)(HIWORD(lParam) * 0.7), 
			(int)(LOWORD(lParam) * 0.7),(int)(HIWORD(lParam) * 0.3), TRUE);
		MoveWindow(hwndInstruction, (int)(LOWORD(lParam) * 0.7), 0, (int)(LOWORD(lParam) * 0.3), 
			(int)(HIWORD(lParam) * 0.5), TRUE);
		MoveWindow(hwndStack, (int)(LOWORD(lParam) * 0.7), (int)(HIWORD(lParam) * 0.5), 
			(int)(LOWORD(lParam) * 0.3), (int)(HIWORD(lParam) * 0.5), TRUE);
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_LOAD:
			if (PopFileOpenDlg(hWnd, szFileName, szTitleName))
			{
				if (!PopFileRead(hwndEdit, szFileName))
				{
					OkMessage(hWnd, TEXT("Could not read file %s!"), szTitleName);
					szFileName[0] = '\0';
					szTitleName[0] = '\0';
				}
			}
			DoCaption(hWnd, szTitleName);
			break;
		case IDM_BUILD:
		{
			EnableMenuItem(hMenu, IDM_RUN, MF_GRAYED);
			EnableMenuItem(hMenu, IDM_DEBUG, MF_GRAYED);
			EnableMenuItem(hMenu, IDM_NEXT, MF_GRAYED);
			EnableMenuItem(hMenu, IDM_TABLE, MF_GRAYED);
			int length = GetWindowTextLengthA(hwndEdit);
			char *content = (char*)malloc(length + 1);
			GetWindowTextA(hwndEdit, content, length + 1);
			SetWindowTextA(hwndInstruction, "INSTRUCTION:");
			SetWindowTextA(hwndStack, "STACK:");
			//content[length] = '\0';
			string codeString = content;
			free(content);
			if (!x0.init(codeString))
			{
				SetWindowTextA(hwndOutput, x0.getResult().c_str());
			}
			else
			{
				if (x0.program())
				{
					EnableMenuItem(hMenu, IDM_RUN, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_DEBUG, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_TABLE, MF_ENABLED);
				}
				SetWindowTextA(hwndOutput, x0.getResult().c_str());
			}
			//SetWindowTextA(hwndOutput, content);
		}
		break;
		case IDM_TABLE:
			SetWindowTextA(hwndOutput, x0.getSymTable().c_str());
			break;
		case IDM_RUN:
			inputString = "";
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT), hWnd, InputDlgProc);
			x0.setInput(inputString);
			if (x0.interpret(output, errorMessage))
			{
				SetWindowTextA(hwndOutput, output.c_str());
			}
			else
			{
				SetWindowTextA(hwndOutput, errorMessage.c_str());
			}
			break;
		case IDM_DEBUG:
			inputString = "";
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT), hWnd, InputDlgProc);
			x0.setInput(inputString);
			output = "===============Start X0===============\r\n";
			pc = 0;
			top = -1;
			inputIdx = 0;
			SetWindowTextA(hwndOutput, output.c_str());
			SetWindowTextA(hwndInstruction, x0.getCodes(pc).c_str());
			SetWindowTextA(hwndStack, x0.getStackStat(top).c_str());
			EnableMenuItem(hMenu, IDM_NEXT, MF_ENABLED);
			break;
		case IDM_NEXT:
			if (!x0.next(pc, output, top, inputIdx, errorMessage))
			{
				SetWindowTextA(hwndOutput, errorMessage.c_str());
				EnableMenuItem(hMenu, IDM_NEXT, MF_GRAYED);
			}
			SetWindowTextA(hwndStack, x0.getStackStat(top).c_str());
			SetWindowTextA(hwndInstruction, x0.getCodes(pc).c_str());
			if (pc == 0)
			{
				output += "===============End X0================\r\n";
				EnableMenuItem(hMenu, IDM_NEXT, MF_GRAYED);
			}
			SetWindowTextA(hwndOutput, output.c_str());
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// “输入”框的消息处理程序
INT_PTR CALLBACK InputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndInput;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		hwndInput = GetDlgItem(hDlg, IDC_INPUT);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			int length = GetWindowTextLengthA(hwndInput);
			char *content = (char*)malloc(length + 1);
			GetWindowTextA(hwndInput, content, length + 1);
			inputString = content;
			free(content);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void PopFileInitialize(HWND hwnd)
{
	static TCHAR szFilter[] = TEXT("Text Files (*.TXT)\0*.txt\0")\
		TEXT("ASCII Files (*.ASC)\0*.asc\0")\
		TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = NULL; // Set in Open and Close functions
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL; // Set in Open and Close functions
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0; // Set in Open and Close functions
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("txt");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
}

BOOL PopFileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
	return GetOpenFileName(&ofn);
}

BOOL PopFileRead(HWND hwndEdit, PTSTR pstrFileName)
{
	BYTE bySwap;
	DWORD dwBytesRead;
	HANDLE hFile;
	int i, iFileLength, iUniTest;
	PBYTE pBuffer, pText, pConv;

	// Open the file.
	if (INVALID_HANDLE_VALUE == 
		(hFile = CreateFile(pstrFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)))
		return FALSE;

	// Get file size in bytes and allocate memory for read.
	// Add an extra two bytes for zero termination.
	iFileLength = GetFileSize(hFile, NULL);
	pBuffer = (PBYTE)malloc(iFileLength + 2);

	// Read file and put terminating zeros at end.
	ReadFile(hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
	CloseHandle(hFile);
	pBuffer[iFileLength] = '\0';
	pBuffer[iFileLength + 1] = '\0';

	// Test to see if the text is Unicode
	iUniTest = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;
	if (IsTextUnicode(pBuffer, iFileLength, &iUniTest))
	{
		pText = pBuffer + 2;
		iFileLength -= 2;
		if (iUniTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE)
		{
			for (i = 0; i < iFileLength / 2; i++)
			{
				bySwap = ((BYTE *)pText)[2 * i];
				((BYTE *)pText)[2 * i] = ((BYTE *)pText)[2 * i + 1];
				((BYTE *)pText)[2 * i + 1] = bySwap;
			}
		}

		// Allocate memory for possibly converted string
		pConv = (PBYTE)malloc(iFileLength + 2);

		// If the edit control is not Unicode, convert Unicode text to
		// non-Unicode (i.e., in general, wide character).
#ifndef UNICODE
		WideCharToMultiByte(CP_ACP, 0, (PWSTR)pText, -1, pConv, iFileLength + 2, NULL, NULL);
		// If the edit control is Unicode, just copy the string
#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif
	}
	else // the file is not Unicode
	{
		pText = pBuffer;
		// Allocate memory for possibly converted string.
		pConv = (PBYTE)malloc(2 * iFileLength + 2);
		// If the edit control is Unicode, convert ASCII text.
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, (LPCCH)pText, -1, (PTSTR)pConv, iFileLength + 1);
		// If not, just copy buffer
#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif
	}
	SetWindowText(hwndEdit, (PTSTR)pConv);
	free(pBuffer);
	free(pConv);
	return TRUE;
}
