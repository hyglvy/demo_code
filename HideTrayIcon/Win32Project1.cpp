// Win32Project1.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <CommCtrl.h>
#include <iostream>

BOOL IsWow64()
{
	BOOL bIsWow64 = FALSE;
	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = reinterpret_cast<LPFN_ISWOW64PROCESS>(::GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process"));
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(::GetCurrentProcess(), &bIsWow64))
		{
			//handle error
		}
	}
	return bIsWow64;
}

bool IsWindowsVersionOrGreater_(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0 };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

bool IsWindowsVistaOrGreater_()
{
	return IsWindowsVersionOrGreater_(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
}

HWND _FindOverflowToolbarWnd(void)
{
	HWND hToolBar = FindWindow(_T("NotifyIconOverflowWindow"), NULL);
	if (hToolBar == NULL)
		return NULL;
	return FindWindowEx(hToolBar, NULL, TOOLBARCLASSNAME, NULL);
}

HWND _FindToolbar32Wnd(void)
{
	HWND hToolBar = FindWindow(_T("Shell_TrayWnd"), NULL);
	if (hToolBar == NULL)
		return NULL;
	hToolBar = FindWindowEx(hToolBar, NULL, _T("TrayNotifyWnd"), NULL);
	if (hToolBar == NULL)
		return NULL;
	HWND hSyspagerWnd = FindWindowEx(hToolBar, NULL, _T("SysPager"), NULL);
	if (hSyspagerWnd == NULL)
		return NULL;
	return FindWindowEx(hSyspagerWnd != NULL ? hSyspagerWnd : hToolBar, NULL, TOOLBARCLASSNAME, NULL);
}

void ShowTrayIcon(HWND hToolbar,LPCTSTR lpwszIcon, BOOL bShow)
{
	//���ݴ��ھ����ȡ����id
	DWORD dwProcessId;
	::GetWindowThreadProcessId(hToolbar, &dwProcessId);

	//����ͼ������
	int nButtonCount = SendMessage(hToolbar, TB_BUTTONCOUNT, 0L, 0L);

	//�򿪽���
	HANDLE hProcess = ::OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, dwProcessId);

	//�����ڴ�
	LPVOID lpTbbution = ::VirtualAllocEx(hProcess, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);

	//��ť��Ϣ
	TBBUTTON tButton;

	for (int i = 0; i < nButtonCount; i++)
	{
		//��ȡTBBUTTON��Ϣ
		SendMessage(hToolbar, TB_GETBUTTON, i, (LPARAM)lpTbbution);
		ReadProcessMemory(hProcess, lpTbbution, &tButton, sizeof(TBBUTTON), NULL);

		//��ȡ���ⳤ��
		DWORD nDesireLen = SendMessage(hToolbar, TB_GETBUTTONTEXT, tButton.idCommand, NULL);
		//���ⲿ���������ڴ�
		LPVOID lpTextAddress = ::VirtualAllocEx(hProcess, NULL, nDesireLen, MEM_COMMIT, PAGE_READWRITE);
		//��ȡ�������ݵ��ⲿ�����ڴ�
		SendMessage(hToolbar, TB_GETBUTTONTEXT, tButton.idCommand, (LPARAM)lpTextAddress);

		//�����ⲿ���̱���ı�������
		TCHAR strTitle[MAX_PATH + 1] = _T("");
		ZeroMemory(strTitle, sizeof(strTitle));
		ReadProcessMemory(hProcess, lpTextAddress, &strTitle, sizeof(strTitle), NULL);

		//�ͷ���ʱ���������ڴ�
		::VirtualFreeEx(hProcess, lpTextAddress, nDesireLen, MEM_FREE);

		_tprintf(_T("idCommand : %d , title : %s \n"), tButton.idCommand, strTitle);

		if (_tcsstr(strTitle, lpwszIcon))
		{
			_tprintf(_T("idCommand : %d , title : %s \n"), tButton.idCommand, strTitle);

			BOOL bHide = (BOOL)SendMessage(hToolbar, TB_ISBUTTONHIDDEN, (WPARAM)tButton.idCommand, 0L);
			BOOL bResult = (BOOL)SendMessage(hToolbar, TB_HIDEBUTTON, (WPARAM)tButton.idCommand, MAKELPARAM(!bHide, 0));

			_tprintf(_T("bResult : %s \n"), bResult ? _T("TRUE") : _T("FALSE"));
		}
	}

	//�ͷ��ڴ�
	::VirtualFreeEx(hProcess, lpTbbution, sizeof(TBBUTTON), MEM_FREE);
	CloseHandle(hProcess);
}

int main()
{
	setlocale(LC_ALL, "CHS");

	TCHAR *szTitle = _T("������");

	//�����������������û�ڴ�й©
	for (size_t i = 0; i < 1000; i++)
	{
		HWND hToolbar = _FindToolbar32Wnd();
		if (hToolbar != NULL)
		{
			ShowTrayIcon(hToolbar, szTitle, FALSE);
			if (IsWindowsVistaOrGreater_())
			{
				hToolbar = _FindOverflowToolbarWnd();
				if (hToolbar != NULL)
				{
					ShowTrayIcon(hToolbar, szTitle, FALSE);
				}
			}
		}
		Sleep(1500);
	}
		
	getchar();
	return 0;
}

