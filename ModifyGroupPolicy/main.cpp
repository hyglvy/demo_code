#include <windows.h>
#include "InitGuid.h"
#include "gpedit.h"
#include <UserEnv.h>
#pragma comment(lib, "Userenv.lib")

bool SetGroupPolicy(HKEY hKey, LPCTSTR lpszSubKey, LPCTSTR valueName, DWORD dwType, LPCTSTR lpszValue, DWORD dwValue)
{
	::CoInitialize(NULL);

	HRESULT hr = S_OK;
	IGroupPolicyObject *pGPO = NULL;
	HKEY ghKey, ghSubKey, hSubKey;
	LPDWORD flag = NULL;

	hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL, CLSCTX_ALL, IID_IGroupPolicyObject, (LPVOID*)&pGPO);
	if (!SUCCEEDED(hr))
	{
		//MessageBox(NULL, L"GPO接口对象初始化失败", L"", MB_OK);
		::CoUninitialize();
		return false;
	}

	//修改本地注册表
	if (RegCreateKeyEx(hKey, lpszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, flag) != ERROR_SUCCESS) {
		//MessageBox(NULL, L"RegCreateKeyEx 失败", L"", MB_OK);
		::CoUninitialize();
		return false;
	}
	if (dwType == REG_SZ) {
		if (RegSetValueEx(hSubKey, valueName, NULL, dwType, (BYTE *)lpszValue, (_tcslen(lpszValue) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
			RegCloseKey(hSubKey);
			::CoUninitialize();
			return false;
		}
	}
	else if (dwType == REG_DWORD) {
		if (RegSetValueEx(hSubKey, valueName, 0, dwType, (BYTE *)&dwValue, sizeof(dwValue)) != ERROR_SUCCESS) {
			RegCloseKey(hSubKey);
			::CoUninitialize();
			return false;
		}
	}

	//OpenLocalMachine
	if (pGPO->OpenLocalMachineGPO(GPO_OPEN_LOAD_REGISTRY) != S_OK) {
		//MessageBox(NULL, L"获取本地GPO映射失败", L"", MB_OK);
		::CoUninitialize();
		return false;
	}

	//GetRegistryKey
	if (pGPO->GetRegistryKey(GPO_SECTION_USER, &ghKey) != S_OK) {
		//MessageBox(NULL, L"获取本地GPO映射注册表根键失败", L"", MB_OK);
		::CoUninitialize();
		return false;
	}

	if (RegCreateKeyEx(ghKey, lpszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &ghSubKey, flag) != ERROR_SUCCESS) {
		RegCloseKey(ghKey);
		//MessageBox(NULL, L"无法创建组策略注册表子项目", L"", MB_OK);
		::CoUninitialize();
		return false;
	}
	if (dwType == REG_SZ) {
		if (RegSetValueEx(ghSubKey, valueName, 0, dwType, (BYTE *)lpszValue, (_tcslen(lpszValue) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
			RegCloseKey(ghKey);
			RegCloseKey(ghSubKey);
			//MessageBox(NULL, L"无法创建组策略注册表子项目值", L"", MB_OK);
			::CoUninitialize();
			return false;
		}
	}
	else if (dwType == REG_DWORD) {
		if (RegSetValueEx(ghSubKey, valueName, 0, dwType, (BYTE*)&dwValue, sizeof(dwValue)) != ERROR_SUCCESS) {
			RegCloseKey(ghKey);
			RegCloseKey(ghSubKey);
			//MessageBox(NULL, L"无法创建组策略注册表子项目值", L"", MB_OK);
			::CoUninitialize();
			return false;
		}
	}

	GUID RegistryId = REGISTRY_EXTENSION_GUID;
	if (pGPO->Save(false, true, const_cast<GUID*>(&RegistryId), const_cast<GUID*>(&CLSID_GPESnapIn)) != S_OK) {
		RegCloseKey(ghKey);
		RegCloseKey(ghSubKey);
		//MessageBox(NULL, L"保存组策略失败", L"", MB_OK);
		::CoUninitialize();
		return false;
	}
	pGPO->Release();
	RegCloseKey(ghKey);
	RegCloseKey(ghSubKey);
	::CoUninitialize();
	return ::RefreshPolicy(TRUE);
}
