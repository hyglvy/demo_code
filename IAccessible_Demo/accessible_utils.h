/*
* Author: LowBoyTeam (https://github.com/LowBoyTeam)
* License: Code Project Open License
* Disclaimer: The software is provided "as-is". No claim of suitability, guarantee, or any warranty whatsoever is provided.
* Copyright (c) 2016-2017.
*/

#ifndef accessibility_util_h__
#define accessibility_util_h__

#include <string>

#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <oleacc.h>
#pragma comment(lib,"oleacc.lib")

#define SAFE_RELEASE(p) { if (p != NULL) p->Release(); p = NULL; }

typedef BOOL(CALLBACK* ENUMACCESSIBLEPROC)(__in CComPtr<IAccessible>, __in CComVariant, HWND, INT, LPARAM);

namespace accessibility_util
{
	std::wstring GetObjectName(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"<Error>";

		CComBSTR bstrName;
		HRESULT hr = pAcc->get_accName(varChild, &bstrName);

		if (FAILED(hr))
			return L"<Error>";

		if (!bstrName.m_str)
			return L"<NULL>";

		return bstrName.m_str;
	}

	std::wstring GetObjectValue(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"<Error>";

		CComBSTR bstrValue;
		HRESULT hr = pAcc->get_accValue(varChild, &bstrValue);

		if (FAILED(hr))
			return L"<Error>";

		if (!bstrValue.m_str)
			return L"<NULL>";

		return bstrValue.m_str;
	}

	std::wstring GetObjectRoleString(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"<Error>";

		CComVariant varRoleID;
		HRESULT hr = pAcc->get_accRole(varChild, &varRoleID);

		if (FAILED(hr))
			return L"<Error>";

		WCHAR sRoleBuff[1024] = { 0 };
		hr = ::GetRoleText(varRoleID.lVal, sRoleBuff, 1024);

		if (FAILED(hr))
			return L"<Error>";

		return sRoleBuff;
	}

	LONG GetObjectRole(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return -1;

		CComVariant varRole;
		HRESULT hr = pAcc->get_accRole(varChild, &varRole);

		if (FAILED(hr))
			return -1;

		return varRole.lVal;
	}

	std::wstring GetObjectDescription(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"<Error>";

		CComBSTR bstrDesc;
		HRESULT hr = pAcc->get_accDescription(varChild, &bstrDesc);

		if (FAILED(hr))
			return L"<Error>";

		if (!bstrDesc.m_str)
			return L"<NULL>";

		return bstrDesc.m_str;
	}

	LONG GetObjectState(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return -1;

		CComVariant varState;
		HRESULT hr = pAcc->get_accState(varChild, &varState);

		if (FAILED(hr))
			return -1;

		return varState.intVal;
	}

	BOOL GetObjectLocation(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild, RECT& rect)
	{
		if (!pAcc)
			return FALSE;

		HRESULT hr = pAcc->accLocation(&rect.left, &rect.top, &rect.right, &rect.bottom, varChild);

		if (SUCCEEDED(hr))
		{
			// accLocation returns width and height  
			rect.right += rect.left;
			rect.bottom += rect.top;
			return TRUE;
		}

		return FALSE;
	}

	BOOL FindChild(__in CComPtr<IAccessible> pAccParent, CComPtr<IAccessible> pAccChild, __in CComVariant varChild, ENUMACCESSIBLEPROC lpEnumAccessibleProc, LPARAM lParam, BOOL bFirstTime)
	{
		if (!pAccParent || !lpEnumAccessibleProc)
			return FALSE;

		BOOL bContinue = TRUE;
		static int nLevel = 0;

		// call enum callback,if return FALSE so stop enum;
		if (bFirstTime)
		{
			nLevel = 0;
			varChild.vt = VT_I4;
			varChild.lVal = CHILDID_SELF;
			bContinue = lpEnumAccessibleProc(pAccParent, varChild, NULL, nLevel, lParam);
		}

		nLevel++;

		CComPtr<IEnumVARIANT> pEnum;
		HRESULT hr = pAccParent->QueryInterface(IID_IEnumVARIANT, (PVOID*)&pEnum);
		if (SUCCEEDED(hr) && pEnum) {
			pEnum->Reset();
		}

		// get child count  
		long nChildren = 0;
		unsigned long nFetched = 0;

		pAccParent->get_accChildCount(&nChildren);

		for (long index = 1; (index <= nChildren) && bContinue; index++)
		{
			varChild.Clear();

			if (pEnum)
			{
				hr = pEnum->Next(1, &varChild, &nFetched);
				if (FAILED(hr))
				{
					bContinue = FALSE;
					break;
				}
			}
			else
			{
				varChild.vt = VT_I4;
				varChild.lVal = index;
			}

			// get IDispatch interface for the child  
			CComPtr<IDispatch> pDisp;
			if (VT_I4 == varChild.vt) {
				hr = pAccParent->get_accChild(varChild, &pDisp);
			}
			else if (VT_DISPATCH == varChild.vt) {
				pDisp = varChild.pdispVal;
			}

			// get IAccessible interface for the child  
			CComPtr<IAccessible> pCAcc;
			if (NULL != pDisp) {
				hr = pDisp->QueryInterface(IID_IAccessible, (PVOID*)&pCAcc);
				if (FAILED(hr)) {
					continue;
				}
			}

			// get information about the child  
			if (NULL != pCAcc) {
				varChild.Clear();
				varChild.vt = VT_I4;
				varChild.lVal = CHILDID_SELF;
				pAccChild = pCAcc;
			}
			else {
				pAccChild = pAccParent;
			}

			DWORD dwState = GetObjectState(pAccChild, varChild);

			// check if object is available  
			if (dwState == -1 || (dwState & STATE_SYSTEM_INVISIBLE))
				continue;

			HWND hwndChild = 0;
			WindowFromAccessibleObject(pAccChild, &hwndChild);

			// call enum callback
			bContinue = lpEnumAccessibleProc(pAccChild, varChild, hwndChild, nLevel, lParam);

			if (bContinue && pCAcc)
				bContinue = FindChild(pCAcc, pAccChild, varChild, lpEnumAccessibleProc, lParam, FALSE);
		}

		nLevel--;

		return bContinue;
	}

	BOOL EnumAccessible(HWND hwnd, ENUMACCESSIBLEPROC lpEnumAccessibleProc, LPARAM lParam)
	{
		BOOL bRet = FALSE;
		if (::IsWindow(hwnd) && lpEnumAccessibleProc)
		{
			CoInitialize(NULL);
			CComPtr<IAccessible> pIAcc;
			HRESULT hr = ::AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible, (PVOID*)&pIAcc);
			if (SUCCEEDED(hr) && pIAcc)
			{
				CComVariant varChild;
				CComPtr<IAccessible> pIAccChild;
				FindChild(pIAcc, pIAccChild, varChild, lpEnumAccessibleProc, NULL,TRUE);
				bRet = TRUE;
			}
			CoUninitialize();
		}
		return bRet;
	}
}

#endif // accessibility_util_h__

