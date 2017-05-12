#include <Windows.h>
#include <tchar.h>
#include <string>

int main(int argc, _TCHAR* argv[])
{
	typedef VOID(*InstallSvc)();
	HINSTANCE hinstLib2 = LoadLibrary(TEXT("SampleSvcHostService.dll"));
	InstallSvc _InstallSvc = NULL;
	_InstallSvc = (InstallSvc)GetProcAddress(hinstLib2, "InstallSvc");
	//_InstallSvc = (InstallSvc)GetProcAddress(hinstLib2, "UninstallSvc");	
	_InstallSvc();
	system("net start CppWindowsService");
	getchar();
	return 0;
}