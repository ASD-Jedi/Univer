// MathLibrary.cpp : Defines the exported functions for the DLL application.
// Compile by using: cl /EHsc /DMATHLIBRARY_EXPORTS /LD MathLibrary.cpp

#include "stdafx.h"
#include "MathLibrary.h"

namespace MathLibrary
{
	double Functions::Add(double a, double b)
	{
		return a + b;
	}

	double Functions::Multiply(double a, double b)
	{
		return a * b;
	}

	double Functions::AddMultiply(double a, double b)
	{
		return a + (a * b);
	}

	//lParam should be a pointer to hookStruct, that was casted to LPARAM: (LPARAM)&hookStruct
	LRESULT CALLBACK Functions::Zero(int nCode, WPARAM wParam, LPARAM lParam) {
		ZeroHook* hookStruct = (ZeroHook*)lParam;
		MessageBox(NULL, L"Hello", L"TEST", MB_OK);
		return CallNextHookEx(hookStruct->hook, nCode, wParam, lParam);
	}
}