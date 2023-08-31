// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

// 여기에 미리 컴파일하려는 헤더 추가
#include "framework.h"


#endif //PCH_H
#pragma once 

#define NO_WARN_MBCS_MFC_DEPRECATION

#include <map>
#include <stack>
#include <deque>
#include <list>
#include <vector>
#include <queue>



#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;


#define COINITIALIZEEX_MULTI_THREADED \
   BOOL __bCoInitialized__ = FALSE; \
   HRESULT __hResult__ = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED ); \
   if (__hResult__ == S_OK) { \
      __bCoInitialized__ = TRUE; \
   } \
   else if (__hResult__ == RPC_E_CHANGED_MODE ) { \
      ASSERT (0); \
   } \

#define COUNINITIALIZEEX_MULTI_THREADED \
   if (__bCoInitialized__) { \
      CoUninitialize (); \
      __bCoInitialized__ = FALSE; \
   } \


#ifndef	SAFE_DELETE
#define	SAFE_DELETE(x) if(x) { delete x; x = NULL; }
#endif


#include <locale.h>
#define TRACE_NORMAL			0x00001
#define TRACE_INFO				0x01400
#define TRACE_ERROR				0x01800

static void SendLog(DWORD dwType, LPCTSTR lpDebugStr, ...)
{
    setlocale(LC_ALL, "korean");

    va_list argList;
    va_start(argList, lpDebugStr);


    USES_CONVERSION;

    CString strFormat;
    strFormat = lpDebugStr;

    CString csDebugStr;
    csDebugStr.FormatV(strFormat, argList);

    va_end(argList);

    static HWND pWnd = NULL;

    if (NULL == pWnd)
        pWnd = FindWindow(NULL, _T("NDR Logger"));

    if (NULL != pWnd)
    {
        COPYDATASTRUCT cds;
        cds.dwData = dwType;
        cds.cbData = csDebugStr.GetLength() * 2;
        cds.lpData = (LPSTR)(LPCTSTR)A2W(csDebugStr);

        ::SendMessage(pWnd, WM_COPYDATA, NULL, (LPARAM)&cds);

    }
}