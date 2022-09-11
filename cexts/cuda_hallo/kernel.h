#pragma once

#ifdef CUDA_HALLO_EXPORTS
	#define CUDA_HALLO_API __declspec(dllexport)
#else
	#define CUDA_HALLO_API
#endif

void CudaTestFunc();
