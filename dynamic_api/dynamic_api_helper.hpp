/*
* Author: LowBoyTeam (https://github.com/LowBoyTeam)
* License: Code Project Open License
* Disclaimer: The software is provided "as-is". No claim of suitability, guarantee, or any warranty whatsoever is provided.
* Copyright (c) 2016-2017.
*/

#ifndef _DYNAMIC_API_HELPER_H__
#define _DYNAMIC_API_HELPER_H__

#include <windows.h>
#include <assert.h>

#define TOSTRING(x) #x
#define TOWSTRING(x) L#x
#define NAME_CAT(prefix, x) prefix##x

#define BEGIN_API_MAP(name)																				\
do																										\
{																										\
NAME_CAT(m_h, name) = ::LoadLibraryW(TOWSTRING(name));													\
HMODULE hMod = NAME_CAT(m_h, name);																		\
assert(hMod != NULL);
#define API_ITEM(proc)																					\
NAME_CAT(m_pfn, proc) = reinterpret_cast<NAME_CAT(PFN, proc)> (::GetProcAddress(hMod, TOSTRING(proc))); \
assert(NAME_CAT(m_pfn, proc) != NULL);
#define END_API_MAP() }while(0);
#define API_DEFINE(proc) NAME_CAT(PFN, proc) NAME_CAT(m_pfn, proc)
#define MOD_DEFINE(name) HMODULE NAME_CAT(m_h, name)
#define MOD_RELEASE(name) m_pfnFreeLibrary(NAME_CAT(m_h, name))
#define STDCL_T(ret, proc) typedef ret (WINAPI *NAME_CAT(PFN, proc))
#define TYPEDEF(callconv, proc) (callconv *NAME_CAT(PFN, proc))
#define _API_(proc) Api::GetInstance().NAME_CAT(m_pfn, proc)

class Api
{
public:

	//singleton
	static Api& GetInstance()
	{
		static Api instance;
		return instance;
	}

	Api()
	{
		BEGIN_API_MAP(Kernel32)
			API_ITEM(GetModuleFileNameW)
			API_ITEM(FreeLibrary)
			API_ITEM(CloseHandle)
			API_ITEM(GetCommandLineW)
			API_ITEM(GetTempPathA)
			API_ITEM(GetTempFileNameA)
			API_ITEM(WaitForSingleObject)
			API_ITEM(Sleep)
		END_API_MAP()

		BEGIN_API_MAP(User32)
			API_ITEM(FindWindowW)
			API_ITEM(FindWindowExW)
			API_ITEM(GetWindowThreadProcessId)
			API_ITEM(IsWindow)
			API_ITEM(SendMessageW)
			API_ITEM(SetWindowPos)
		END_API_MAP()
	}

	~Api()
	{
		MOD_RELEASE(User32);
		MOD_RELEASE(Kernel32);
	}

	// kernel32
	STDCL_T(DWORD, GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);
	STDCL_T(BOOL, FreeLibrary)(HMODULE);
	STDCL_T(VOID, CloseHandle)(HANDLE);
	STDCL_T(LPWSTR, GetCommandLineW)(void);
	STDCL_T(DWORD, GetTempPathA)(DWORD, LPSTR);
	STDCL_T(UINT, GetTempFileNameA)(LPCSTR, LPCSTR, UINT, LPSTR);
	STDCL_T(DWORD, WaitForSingleObject)(HANDLE, DWORD);
	STDCL_T(VOID, Sleep)(DWORD);

	API_DEFINE(GetModuleFileNameW);
	API_DEFINE(FreeLibrary);
	API_DEFINE(CloseHandle);
	API_DEFINE(GetCommandLineW);
	API_DEFINE(GetTempPathA);
	API_DEFINE(GetTempFileNameA);
	API_DEFINE(WaitForSingleObject);
	API_DEFINE(Sleep);

	// User32
	STDCL_T(HWND, FindWindowW)(LPCWSTR, LPCWSTR);
	STDCL_T(HWND, FindWindowExW)(HWND, HWND, LPCWSTR, LPCWSTR);
	STDCL_T(DWORD, GetWindowThreadProcessId)(HWND, LPDWORD);
	STDCL_T(BOOL, IsWindow)(HWND);
	STDCL_T(LRESULT, SendMessageW)(HWND, UINT, WPARAM, LPARAM);
	STDCL_T(BOOL, SetWindowPos)(HWND, HWND, int, int, int, int, UINT);

	API_DEFINE(FindWindowW);
	API_DEFINE(FindWindowExW);
	API_DEFINE(GetWindowThreadProcessId);
	API_DEFINE(IsWindow);
	API_DEFINE(SendMessageW);
	API_DEFINE(SetWindowPos);

private:
	MOD_DEFINE(Kernel32);
	MOD_DEFINE(User32);
};

#endif // _DYNAMIC_API_HELPER_H__