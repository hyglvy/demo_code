#include <windows.h>
#include <iostream>
#include "accessible_utils.h"

BOOL CALLBACK EnumAccessibleProc(__in CComPtr<IAccessible> pAccChild, __in CComVariant varChild, HWND hwndChild, int nLevel, LPARAM lParam);

int main()
{
	setlocale(LC_ALL, "CHS");
	HWND hWnd = GetDesktopWindow();//::FindWindow(L"HwndWrapper[DefaultDomain;;0afff95f-c83b-4cd2-acc6-6d571397baa4]", NULL);
	accessibility_util::EnumAccessible(hWnd, EnumAccessibleProc, NULL);
	getchar();
	return 0;
}

BOOL CALLBACK EnumAccessibleProc(__in CComPtr<IAccessible> pAccChild, __in CComVariant varChild, HWND hwndChild, int nLevel, LPARAM lParam)
{
	std::wstring name = accessibility_util::GetObjectName(pAccChild, varChild);
	std::wstring value = accessibility_util::GetObjectValue(pAccChild, varChild);
	std::wstring role = accessibility_util::GetObjectRoleString(pAccChild, varChild);
	std::wstring desc = accessibility_util::GetObjectDescription(pAccChild, varChild);

	RECT rect;
	accessibility_util::GetObjectLocation(pAccChild, varChild, rect);
	LONG nRole = accessibility_util::GetObjectRole(pAccChild, varChild);

	wprintf(L"hwnd = 0x%08X,name = %s,role = %s,value = %s,desc = %s,left = %d,right = %d,top = %d,bottom = %d \n", hwndChild, name.c_str(), role.c_str(), value.c_str(), desc.c_str(), rect.left, rect.right, rect.top, rect.bottom);

	return TRUE;
}