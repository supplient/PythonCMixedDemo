#pragma once

#ifdef HALLO_EXPORTS
	#define HALLO_API __declspec(dllexport)
#else
	#define HALLO_API
#endif

HALLO_API void TestFunc();

