// HideExportTable.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "EnvironmentBlock.h"

using namespace EnvironmentBlock;

//LdrLoadDll function prototype  
typedef NTSTATUS(NTAPI *fLdrLoadDll)(
	IN PWCHAR PathToFile OPTIONAL,
	IN ULONG Flags OPTIONAL,
	IN PUNICODE_STRING ModuleFileName,
	OUT PHANDLE ModuleHandle
	);

//LdrGetProcedureAddress function prototype
typedef NTSTATUS(NTAPI *fLdrGetProcedureAddress)(
	IN PVOID BaseAddress,
	IN PANSI_STRING Name,
	IN ULONG Ordinal,
	OUT PVOID *ProcedureAddress);

//LdrUnloadDll function prototype
typedef NTSTATUS(NTAPI *fLdrUnloadDll)(
	IN PVOID BaseAddress);

//RtlInitUnicodeString function prototype
typedef VOID(WINAPI *fRtlInitUnicodeString)(
	PUNICODE_STRING DestinationString,
	PCWSTR SourceString);

typedef int(WINAPI *fMessageBoxA)(HWND, PSTR, PSTR, UINT);


DWORD GetExportTableProcAddress(HMODULE hModule, LPSTR lpProcName) {

	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNtHeader = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDir = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = NULL;

	pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
	pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>((DWORD)pDosHeader + pDosHeader->e_lfanew);

	//����������
	pExportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>((DWORD)hModule + pNtHeader->OptionalHeader.DataDirectory[0].VirtualAddress);
	//�����������Ʊ�
	PDWORD nameTable = reinterpret_cast<PDWORD>((DWORD)hModule + pExportDir->AddressOfNames);
	//����������ַ��
	PDWORD funcTable = reinterpret_cast<PDWORD>((DWORD)hModule + pExportDir->AddressOfFunctions);
	//��ű�
	PWORD nameOrdinalTable = reinterpret_cast<PWORD>((DWORD)hModule + pExportDir->AddressOfNameOrdinals);

	//DWORD *dwAddress;
	//PSTR pName = NULL;
	//WORD swOrdinal;
	//for (int i = 0; i < pExportDir->NumberOfNames; i++) {//i����Hint
	//	pName = (PSTR)((DWORD)hModule + nameTable[i]);
	//	printf("%s\n", pName);
	//	int iRet = strcmp(lpProcName, pName);
	//	if (iRet == 0) {
	//		swOrdinal = nameOrdinalTable[i];//ȡ��Oridinal
	//		dwAddress = (DWORD*)((DWORD)hModule + funcTable[swOrdinal]);//ͨ��Oridinal��õ�ַ
	//		break;
	//	}
	//}
	//return dwAddress;

	//��������������, ���ֲ��ҷ�,�ٶȸܸܵ�
	DWORD right = pExportDir->NumberOfNames;
	DWORD left = 0;
	DWORD mid;
	PSTR pName = NULL;
	int cmpResult;
	while (left <= right)
	{
		mid = (left + right) >> 1;
		pName = (PSTR)((DWORD)hModule + nameTable[mid]);
		printf("%s\n", pName);
		cmpResult = strcmp(lpProcName, pName);
		if (!cmpResult)
		{
			return (DWORD)hModule + funcTable[nameOrdinalTable[mid]];
		}
		if (cmpResult < 0)
		{
			right = mid - 1;
		}
		else
		{
			left = mid + 1;
		}
	}
	return NULL;
}

int main()
{
	
	getchar();
	
	HMODULE hModule = FindLoadedModuleHandle(L"ntdll.dll");

	fLdrLoadDll _LdrLoadDll;
	fLdrGetProcedureAddress _LdrGetProcedureAddress;
	fLdrUnloadDll _LdrUnloadDll;
	fRtlInitUnicodeString _RtlInitUnicodeString;

	_LdrLoadDll = (fLdrLoadDll)GetExportTableProcAddress(hModule, "LdrLoadDll");
	_LdrGetProcedureAddress = (fLdrGetProcedureAddress)GetExportTableProcAddress(hModule, "LdrGetProcedureAddress");
	_LdrUnloadDll = (fLdrUnloadDll)GetExportTableProcAddress(hModule, "LdrUnloadDll");
	_RtlInitUnicodeString = (fRtlInitUnicodeString)GetExportTableProcAddress(hModule, "RtlInitUnicodeString");

	HANDLE hUser32Module = NULL;
	UNICODE_STRING DllName;
	_RtlInitUnicodeString(&DllName, L"user32.dll");
	int iRet = _LdrLoadDll(0, 0, &DllName, &hUser32Module);

	fMessageBoxA _MessageBoxA = (fMessageBoxA)GetExportTableProcAddress((HMODULE)hUser32Module, "MessageBoxA");
	_MessageBoxA(NULL, "This is Caption", "This is Text", MB_OK);
	iRet = _LdrUnloadDll(hUser32Module);

	getchar();
	return 0;
}