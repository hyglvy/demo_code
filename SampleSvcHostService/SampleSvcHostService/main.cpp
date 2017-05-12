#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define WSTR_SIZE(s) (wcslen(s) * sizeof(wchar_t));
#define TSTR_SIZE(s) ((_tcslen(p) + 1) * sizeof(TCHAR));

#define SERVER_GROUP_NAME			L"MyServiceGroupEx"
// Internal name of the service
#define SERVICE_NAME				L"CppWindowsService"
// Displayed name of the service
#define SERVICE_DISPLAY_NAME		L"CppWindowsService Sample Service"
// The status of the service
SERVICE_STATUS m_status;
// The service status handle
SERVICE_STATUS_HANDLE m_statusHandle;

HMODULE		m_hModule;

// Set the service status and report the status to the SCM.
void SetServiceStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode = NO_ERROR,DWORD dwWaitHint = 0);
VOID WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);
void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
void OnStart(DWORD dwArgc, LPWSTR *lpszArgv);
void OnStop();
void OnPause();
void OnContinue();
void OnShutdown();
DWORD WINAPI ServiceWorkerThread(LPVOID lparam);

// TRACE sends a string to the debug/output pane, or an external debugger
inline void TRACE(LPCTSTR str, ...)
{
	va_list args;
	va_start(args, str);
	TCHAR szData[1024] = { 0 };
	_vsntprintf_s(szData, sizeof(szData) - 1, str, args);
	va_end(args);
#ifdef  _DEBUG
	_tprintf(szData);
#else
	OutputDebugString(szData);
#endif
}

bool IsWinX64()
{
	BOOL bIsWow64 = false;
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = reinterpret_cast<LPFN_ISWOW64PROCESS>(::GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process"));
	if (NULL != fnIsWow64Process)
	{
		fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
	}
	return bIsWow64;
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	m_hModule = hModule;
	return TRUE;
}

void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
	TRACE(L"Enter ServiceMain .....");

	// The service runs in its own process.
	m_status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	// The service is starting.
	m_status.dwCurrentState = SERVICE_STOP_PENDING;
	// The accepted commands of the service.
	m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_CONTROL_CONTINUE;

	m_status.dwWin32ExitCode = NO_ERROR;
	m_status.dwServiceSpecificExitCode = 0;
	m_status.dwCheckPoint = 0;
	m_status.dwWaitHint = 0;

	// Register the handler function for the service
	m_statusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
	
	if (m_statusHandle == NULL)
	{
		TRACE(L"RegisterServiceCtrlHandler failed");
		throw GetLastError();
	}

	// Perform service-specific initialization.
	OnStart(dwArgc, lpszArgv);
}

void SetServiceStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode,DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure of the service.

	m_status.dwCurrentState = dwCurrentState;
	m_status.dwWin32ExitCode = dwWin32ExitCode;
	m_status.dwWaitHint = dwWaitHint;

	m_status.dwCheckPoint =
		((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED)) ?
		0 : dwCheckPoint++;

	// Report the status of the service to the SCM.
	::SetServiceStatus(m_statusHandle, &m_status);
}

void WINAPI ServiceCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP: OnStop(); break;
	case SERVICE_CONTROL_PAUSE: OnPause(); break;
	case SERVICE_CONTROL_CONTINUE: OnContinue(); break;
	case SERVICE_CONTROL_SHUTDOWN: OnShutdown(); break;
	case SERVICE_CONTROL_INTERROGATE: break;
	default: break;
	}
}

void OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
	try
	{
		// Tell SCM that the service is starting.
		SetServiceStatus(SERVICE_START_PENDING);

		TRACE(L"Enter OnStart .....");

		// Queue the main service function for execution in a worker thread.
		CreateThread(NULL, NULL, ServiceWorkerThread, NULL, NULL, NULL);

		// Tell SCM that the service is started.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		TRACE(L"Service Start", dwError);
		// Set the service status to be stopped.
		SetServiceStatus(SERVICE_STOPPED, dwError);
	}
	catch (...)
	{
		// Log the error.
		TRACE(L"Service failed to start.", EVENTLOG_ERROR_TYPE);
		// Set the service status to be stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
}

void OnStop() {
	DWORD dwOriginalState = m_status.dwCurrentState;
	try
	{
		// Tell SCM that the service is stopping.
		SetServiceStatus(SERVICE_STOP_PENDING);

		TRACE(L"Enter OnStop .....");

		// Tell SCM that the service is stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		TRACE(L"Service Stop", dwError);
		// Set the orginal service status.
		SetServiceStatus(dwOriginalState);
	}
	catch (...)
	{
		// Log the error.
		TRACE(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);
		// Set the orginal service status.
		SetServiceStatus(dwOriginalState);
	}
}

void OnPause() {
	try
	{
		// Tell SCM that the service is pausing.
		SetServiceStatus(SERVICE_PAUSE_PENDING);

		TRACE(L"Enter OnPause .....");

		// Tell SCM that the service is paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		TRACE(L"Service Pause", dwError);
		// Tell SCM that the service is still running.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (...)
	{
		// Log the error.
		TRACE(L"Service failed to pause.", EVENTLOG_ERROR_TYPE);
		// Tell SCM that the service is still running.
		SetServiceStatus(SERVICE_RUNNING);
	}
}

void OnContinue() {
	try
	{
		// Tell SCM that the service is resuming.
		SetServiceStatus(SERVICE_CONTINUE_PENDING);

		TRACE(L"Enter OnContinue .....");

		// Tell SCM that the service is running.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		TRACE(L"Service Continue", dwError);
		// Tell SCM that the service is still paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
	catch (...)
	{
		// Log the error.
		TRACE(L"Service failed to resume.", EVENTLOG_ERROR_TYPE);
		// Tell SCM that the service is still paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
}

void OnShutdown() {
	try
	{
		TRACE(L"Enter OnShutdown .....");
		// Tell SCM that the service is stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		TRACE(L"Service Shutdown", dwError);
	}
	catch (...)
	{
		// Log the error.
		TRACE(L"Service failed to shut down.", EVENTLOG_ERROR_TYPE);
	}
}

void CALLBACK InstallSvc() {

	wchar_t wszDllPath[MAX_PATH];
	if (GetModuleFileName(m_hModule, wszDllPath, ARRAY_SIZE(wszDllPath)) == 0)
	{
		TRACE(L"GetModuleFileName failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

	TRACE(L"wszDllPath = %s \n", wszDllPath);

	wchar_t wszBinPath[MAX_PATH] = {0};
	wcscpy_s(wszBinPath, L"%SystemRoot%\\System32\\svchost.exe -k ");
	wcscat_s(wszBinPath, SERVER_GROUP_NAME);

	TRACE(L"wszBinPath = %s \n", wszBinPath);

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	// Open the local default service control manager database
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == NULL)
	{
		TRACE(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

	// Install the service into SCM by calling CreateService
	schService = CreateService(
		schSCManager,                   // SCManager database
		SERVICE_NAME,					// Name of service
		SERVICE_DISPLAY_NAME,			// Name to display
		SERVICE_ALL_ACCESS,				// Desired access
		SERVICE_WIN32_SHARE_PROCESS,    // Service type
		SERVICE_AUTO_START,             // Service start type
		SERVICE_ERROR_NORMAL,           // Error control type
		wszBinPath,                     // Service's binary
		NULL,                           // No load ordering group
		NULL,                           // No tag identifier
		NULL,							// Dependencies
		NULL,							// Service running account
		NULL							// Password of the account
	);

	if (schService == NULL)
	{
		TRACE(L"CreateService failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (schSCManager)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}
	if (schService)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}

	DWORD dwByteLen = WSTR_SIZE(SERVICE_NAME);
	if (IsWinX64())
		SHSetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Svchost", SERVER_GROUP_NAME, REG_MULTI_SZ, SERVICE_NAME, dwByteLen);
	else
		SHSetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost", SERVER_GROUP_NAME, REG_MULTI_SZ, SERVICE_NAME, dwByteLen);

	wchar_t wszSubKey[MAX_PATH] = { 0 };
	wcscpy_s(wszSubKey, L"SYSTEM\\CurrentControlSet\\Services\\");
	wcscat_s(wszSubKey, SERVICE_NAME);
	dwByteLen = WSTR_SIZE(SERVICE_DISPLAY_NAME);
	SHSetValue(HKEY_LOCAL_MACHINE, wszSubKey, L"Description", REG_SZ, SERVICE_DISPLAY_NAME, dwByteLen);

	TRACE(L"wszSubKey = %s ,Description, %s \n", wszSubKey, SERVICE_DISPLAY_NAME);

	wcscat_s(wszSubKey, L"\\Parameters");
	dwByteLen = WSTR_SIZE(wszDllPath);
	SHSetValue(HKEY_LOCAL_MACHINE, wszSubKey, L"ServiceDll", REG_EXPAND_SZ, wszDllPath, dwByteLen);
	TRACE(L"wszSubKey = %s ,ServiceDll, %s \n", wszSubKey, wszDllPath);

	TRACE(L"%s is installed.\n", SERVICE_NAME);
}

void CALLBACK UninstallSvc() {

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};

	// Open the local default service control manager database
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == NULL)
	{
		TRACE(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

	// Open the service with delete, stop, and query status permissions
	schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_STOP |
		SERVICE_QUERY_STATUS | DELETE);
	if (schService == NULL)
	{
		TRACE(L"OpenService failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

	// Try to stop the service
	if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
	{
		TRACE(L"Stopping %s.", SERVICE_NAME);
		Sleep(1000);

		while (QueryServiceStatus(schService, &ssSvcStatus))
		{
			if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				TRACE(L".");
				Sleep(1000);
			}
			else break;
		}

		if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
		{
			TRACE(L"\n%s is stopped.\n", SERVICE_NAME);
		}
		else
		{
			TRACE(L"\n%s failed to stop.\n", SERVICE_NAME);
		}
	}

	// Now remove the service by calling DeleteService.
	if (!DeleteService(schService))
	{
		TRACE(L"DeleteService failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (schSCManager)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}
	if (schService)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}

	if (IsWinX64())
		SHDeleteValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Svchost", SERVER_GROUP_NAME);
	else
		SHDeleteValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost", SERVER_GROUP_NAME);

	wchar_t wszSubKey[MAX_PATH] = { 0 };
	wcscpy_s(wszSubKey, L"SYSTEM\\CurrentControlSet\\Services\\");
	wcscat_s(wszSubKey, SERVICE_NAME);
	SHDeleteKey(HKEY_LOCAL_MACHINE, wszSubKey);

	TRACE(L"%s is removed.\n", SERVICE_NAME);
}

DWORD WINAPI ServiceWorkerThread(LPVOID lparam)
{
	// Periodically check if the service is stopping.
	while (true)
	{
		// Perform main service function here...
		TRACE(L"ServiceWorkerThread .....");

		::Sleep(10000);  // Simulate some lengthy operations.
	}
	return NULL;
}