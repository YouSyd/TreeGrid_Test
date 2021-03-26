#include "TreeGrid.h"

#pragma comment(lib,"TreeGrid.lib")


#ifdef UNITTEST_TREEGRID
/******************************************************************/
/************************        TEST        **********************/
/******************************************************************/
HINSTANCE GInstance;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

void regist_window(char* class_name,HINSTANCE instance,WNDPROC proc)
{
    WNDCLASSEX wndclass;

    wndclass.cbSize=sizeof(wndclass);
    wndclass.style = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = instance;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0,0,0));
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = class_name;
    wndclass.hIconSm = NULL;

    RegisterClassEx(&wndclass);
}

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    GInstance = hInstance;
    char szClassName[]="TreeGrid Test";
    
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    regist_window(YCL_TREEGRID,hInstance,TreeGridOwnProc);
    regist_window(szClassName,hInstance,MainWndProc);
        
    HWND hwnd = CreateWindowEx(0,szClassName,szClassName,
                               WS_THICKFRAME|WS_CAPTION|WS_OVERLAPPEDWINDOW|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CLIPCHILDREN,
                               200,50,700,600,NULL,NULL,hInstance,NULL);
    if(hwnd == NULL) {
        MessageBox(NULL,"create windows error !","error",MB_OK);
        return -1;
    }
    
    ShowWindow(hwnd,nCmdShow);
    UpdateWindow(hwnd);
                                           
    MSG msg;
    while(GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
	GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
}


LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch(message)
    {
    case WM_CREATE: {
#define IDC_TREEGRID 0x0001
        CreateWindowEx(0,YCL_TREEGRID,YCL_TREEGRID,
                       WS_CHILD|WS_VISIBLE|WS_BORDER,
                       0,0,100,100,hwnd,(HMENU)IDC_TREEGRID,(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE),NULL);
    } break;
    case WM_DESTROY: {
		PostQuitMessage(0);
		return (0);
    } break;
    case WM_SIZE: {
        int height=(int)HIWORD(lParam);
        int width=(int)LOWORD(lParam);
        SetWindowPos(GetDlgItem(hwnd,IDC_TREEGRID),NULL,5,5,width-5*2,height-5*2,SWP_NOZORDER|SWP_SHOWWINDOW);
    } break;
    }
    return DefWindowProc(hwnd,message,wParam,lParam);
}

#endif