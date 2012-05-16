//------------------------------------------------------------------------------
// File: DibHelper.cpp
//
// Desc: DirectShow sample code - In-memory push mode source filter
//       Helper routines for manipulating bitmaps.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>

#include "dibhelper.h"

#include <stdio.h>
#include <assert.h>

extern int show_performance;

void logToFile(char *log_this) {
    FILE *f;
	fopen_s(&f, "c:\\temp\\yo2", "a"); // this call fails if using the filter within flash player...
	fprintf(f, log_this);
	fclose(f);
}

// my very own debug output method...
void LocalOutput(const char *str, ...)
{
#ifdef _DEBUG  // avoid in release mode...
  char buf[2048];
  va_list ptr;
  va_start(ptr, str);
  vsprintf_s(buf,str,ptr);
  OutputDebugStringA(buf);
  OutputDebugStringA("\n");
  //logToFile(buf); 
  //logToFile("\n");
  va_end(ptr);
#endif
}

void LocalOutput(const wchar_t *str, ...) 
{
#ifdef _DEBUG  // avoid in release mode...takes like 1ms each message!
  wchar_t buf[2048];
  va_list ptr;
  va_start(ptr, str);
  vswprintf_s(buf,str,ptr);
  OutputDebugString(buf);
  OutputDebugString(L"\n");
  va_end(ptr);
  // also works: OutputDebugString(L"yo ho2");
  //logToFile(buf); 
  //logToFile("\n");
#endif
}

long double PCFreqMillis = 0.0;

// this call only needed once...
// who knows if this is useful or not, speed-wise...
void WarmupCounter()
{
    LARGE_INTEGER li;
	BOOL ret = QueryPerformanceFrequency(&li);
	assert(ret != 0); // only gets run in debug mode LODO
    PCFreqMillis = (long double(li.QuadPart))/1000.0;
}

__int64 StartCounter() // costs 0.0041 ms to do a "start and get" of these...pretty cheap
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (__int64) li.QuadPart;
}

long double GetCounterSinceStartMillis(__int64 sinceThisTime) // see above for some timing
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
	assert(PCFreqMillis != 0.0); // make sure it's been initialized...
    return long double(li.QuadPart-sinceThisTime)/PCFreqMillis; //division kind of forces us to return a double of some sort...
} // LODO do I really need long double here? no really.

// use like 
// 	__int64 start = StartCounter();
// ...
// long double elapsed = GetCounterSinceStartMillis(start)
// printf("took %.020Lf ms", elapsed); // or just use a float and %f or %.02f etc.
// or to debug the method call itself: printf("start %I64d end %I64d took %.02Lf ms", start, StartCounter(), elapsed); 

// partially from http://cboard.cprogramming.com/windows-programming/44278-regqueryvalueex.html

// =====================================================================================
HRESULT RegGetDWord(HKEY hKey, LPCTSTR szValueName, DWORD * lpdwResult) {

	// Given a value name and an hKey returns a DWORD from the registry.
	// eg. RegGetDWord(hKey, TEXT("my dword"), &dwMyValue);

	LONG lResult;
	DWORD dwDataSize = sizeof(DWORD);
	DWORD dwType = 0;

	// Check input parameters...
	if (hKey == NULL || lpdwResult == NULL) return E_INVALIDARG;

	// Get dword value from the registry...
	lResult = RegQueryValueEx(hKey, szValueName, 0, &dwType, (LPBYTE) lpdwResult, &dwDataSize );

	// Check result and make sure the registry value is a DWORD(REG_DWORD)...
	if (lResult != ERROR_SUCCESS) return HRESULT_FROM_WIN32(lResult);
	else if (dwType != REG_DWORD) return DISP_E_TYPEMISMATCH;

	return NOERROR;
}

// =====================================================================================
HRESULT RegGetString(HKEY hKey, LPCTSTR szValueName, LPTSTR * lpszResult) {
 
    // Given a HKEY and value name returns a string from the registry.
    // Upon successful return the string should be freed using free()
    // eg. RegGetString(hKey, TEXT("my value"), &szString);
 
    DWORD dwType=0, dwDataSize=0, dwBufSize=0;
    LONG lResult;
 
    // Incase we fail set the return string to null...
    if (lpszResult != NULL) *lpszResult = NULL;
 
    // Check input parameters...
    if (hKey == NULL || lpszResult == NULL) return E_INVALIDARG;
 
    // Get the length of the string in bytes (placed in dwDataSize)...
    lResult = RegQueryValueEx(hKey, szValueName, 0, &dwType, NULL, &dwDataSize );
 
    // Check result and make sure the registry value is a string(REG_SZ)...
    if (lResult != ERROR_SUCCESS) return HRESULT_FROM_WIN32(lResult);
    else if (dwType != REG_SZ)    return DISP_E_TYPEMISMATCH;
 
    // Allocate memory for string - We add space for a null terminating character...
    dwBufSize = dwDataSize + (1 * sizeof(TCHAR));
    *lpszResult = (LPTSTR) malloc(dwBufSize);
 
    if (*lpszResult == NULL) return E_OUTOFMEMORY;
 
    // Now get the actual string from the registry...
    lResult = RegQueryValueEx(hKey, szValueName, 0, &dwType, (LPBYTE) *lpszResult, &dwDataSize );
 
    // Check result and type again.
    // If we fail here we must free the memory we allocated...
    if (lResult != ERROR_SUCCESS) { free(*lpszResult); return HRESULT_FROM_WIN32(lResult); }
    else if (dwType != REG_SZ)    { free(*lpszResult); return DISP_E_TYPEMISMATCH; }
 
    // We are not guaranteed a null terminated string from RegQueryValueEx.
    // Explicitly null terminate the returned string...
    (*lpszResult)[(dwBufSize / sizeof(TCHAR)) - 1] = TEXT('\0');
 
    return NOERROR;
}

boolean is_config_set_to_1(LPCTSTR szValueName) {
  return read_config_setting(szValueName, 0) == 1;
}

wchar_t output[2048];

wchar_t *read_config_filepath() {
  HKEY hKey;
  LONG i;
  i = RegOpenKeyEx(HKEY_CURRENT_USER,
      L"SOFTWARE\\avisynth-as-dshow-capture",  0, KEY_READ, &hKey);
    
  assert(i==ERROR_SUCCESS); // it's just gotta be there, man!

  LPTSTR stringPointer;
  HRESULT hres = RegGetString(hKey,L"filename_to_read", &stringPointer);
  wcscpy(output, stringPointer);
  free(stringPointer);
  return &output[0];
}


// returns default if nothing is in the registry
 int read_config_setting(LPCTSTR szValueName, int default) {
  HKEY hKey = NULL;
  LONG i;
  i = RegOpenKeyEx(HKEY_CURRENT_USER,
      L"SOFTWARE\\avisynth-as-dshow-capture",  0, KEY_READ, &hKey);
    
  if ( i != ERROR_SUCCESS)
  {	  // assume we don't have to clos eth ekey
    return default;
  } else {
	DWORD dwVal;
	HRESULT hr = RegGetDWord(hKey, szValueName, &dwVal); // works from flash player, standalone...
	RegCloseKey(hKey); // done with that
	if (FAILED(hr)) {
	  return default;
	} else {
		if(dwVal == NOT_SET_IN_REGISTRY) {
			return default;
		} else {
	      return dwVal;
		}
	}
  }
  
 
}

HRESULT set_config_string_setting(LPCTSTR szValueName, wchar_t *szToThis ) {
	
    HKEY hKey = NULL;
    LONG i;    
    DWORD dwDisp = 0;
    LPDWORD lpdwDisp = &dwDisp;

    i = RegCreateKeyEx(HKEY_CURRENT_USER,
       L"SOFTWARE\\avisynth-as-dshow-capture", 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, lpdwDisp); // might still fail in flash player...

    if (i == ERROR_SUCCESS)
    {
		// bad for XP = RegSetKeyValueA(hKey, NULL, szValueName, REG_SZ, ...
        i = RegSetValueEx(hKey, szValueName, 0, REG_SZ, (LPBYTE) szToThis, wcslen(szToThis)*2+1);

    } else {
       // failed to set...
	}

	if(hKey)
	  RegCloseKey(hKey);  // not sure if this could still be NULL...possibly though

	return i;
}
