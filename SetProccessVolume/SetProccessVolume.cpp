// SetProccessVolume.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <mmdeviceapi.h>  
#include <endpointvolume.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <atlbase.h>

BOOL SetProccessVolume(DWORD dwVolume, DWORD dwPid, BOOL IsMixer = TRUE)
{
	HRESULT hr = S_OK;

	CComPtr<IMMDeviceEnumerator>		pIMMEnumerator = NULL;  //主要用于枚举设备接口  
	CComPtr<ISimpleAudioVolume>			pRenderSimpleVol = NULL;  //扬声器的会话音量控制接
	CComPtr<IMMDeviceCollection>		pMultiDevice = NULL;
	CComPtr<IAudioSessionManager2>		pSessionManager = NULL;
	CComPtr<IAudioSessionEnumerator>	pSessionEnumerator = NULL;
	CComPtr<IMMDevice>					pDevice = NULL;

	const IID IID_IMMDeviceEnumerator	= __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioEndpointVolume	= __uuidof(IAudioEndpointVolume);
	const IID IID_ISimpleAudioVolume	= __uuidof(ISimpleAudioVolume);
	const IID IID_IAudioSessionControl2 = __uuidof(IAudioSessionControl2);

	::CoInitialize(NULL);

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pIMMEnumerator);
	if (FAILED(hr))return FALSE;

	if (IsMixer)
		hr = pIMMEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pMultiDevice);
	else
		hr = pIMMEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pMultiDevice);

	if (FAILED(hr)) return FALSE;

	UINT deviceCount = 0;
	hr = pMultiDevice->GetCount(&deviceCount);
	if (FAILED(hr)) return FALSE;

	if ((int)dwVolume < 0) dwVolume = 0;
	if ((int)dwVolume > 100) dwVolume = 100;

	for (UINT ii = 0; ii < deviceCount; ii++)
	{
		pDevice = NULL;
		hr = pMultiDevice->Item(ii, &pDevice);
		if (FAILED(hr)) return FALSE;

		hr = pDevice->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&pSessionManager);
		if (FAILED(hr)) return FALSE;

		hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
		if (FAILED(hr)) return FALSE;

		int nCount;
		hr = pSessionEnumerator->GetCount(&nCount);

		LPWSTR pswSession = NULL;

		for (int i = 0; i < nCount; i++)
		{
			CComPtr<IAudioSessionControl> pSessionControl;
			hr = pSessionEnumerator->GetSession(i, &pSessionControl);
			if (FAILED(hr)) continue;

			hr = pSessionControl->GetDisplayName(&pswSession);
			if (FAILED(hr)) continue;
			//wprintf_s(L"Session Name: %s\n", pswSession);

			CComPtr<IAudioSessionControl2> pSessionControl2;
			hr = pSessionControl->QueryInterface(IID_IAudioSessionControl2, (void**)&pSessionControl2);
			if (FAILED(hr)) continue;

			ULONG pid;
			hr = pSessionControl2->GetProcessId(&pid);
			if (FAILED(hr)) continue;

			CComPtr<ISimpleAudioVolume> pSimpleAudioVolume;
			hr = pSessionControl2->QueryInterface(IID_ISimpleAudioVolume, (void**)&pSimpleAudioVolume);
			if (FAILED(hr)) continue;

			if (pid == dwPid)
			{
				pSimpleAudioVolume->SetMasterVolume((float)dwVolume / 100, NULL);
				::CoUninitialize();
				return TRUE;
			}
		}
	}
	::CoUninitialize();
	return FALSE;
}

int main()
{
	DWORD dwVolume = 100;
	SetProccessVolume(dwVolume, 6512/*pid*/);
	return 0;
}

