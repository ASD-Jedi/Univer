// MathLibrary.h - Contains declaration of Function class
#pragma once
#include <Windows.h>
#ifdef MATHLIBRARY_EXPORTS
#define MATHLIBRARY_API __declspec(dllexport) 
#else
#define MATHLIBRARY_API __declspec(dllimport) 
#endif
typedef struct hookStructZero {
	HHOOK hook;
	int a;
} ZeroHook;
namespace MathLibrary
{
	// This class is exported from the MathLibrary.dll
	class Functions
	{
	public:
		// Returns a + b
		static MATHLIBRARY_API double Add(double a, double b);

		// Returns a * b
		static MATHLIBRARY_API double Multiply(double a, double b);

		// Returns a + (a * b)
		static MATHLIBRARY_API double AddMultiply(double a, double b);
		static MATHLIBRARY_API LRESULT CALLBACK Zero(int nCode, WPARAM wParam, LPARAM lParam);
	};
}