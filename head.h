#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include<gdiplus.h>
using namespace Gdiplus; 

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"gdiplus.lib")

void GradientRect(HDC hdc,RECT rc,COLORREF c1,COLORREF c2,int verical_or_horizen);

typedef struct _RSTRUCT_HEADINFO_ {
    int offset;
    BOOL dragging;
    int dragpt;
    int dragindex;
    int dragnow;
    
    WNDPROC pre_proc;
    WNDPROC proc;
    
    COLORREF color1;//背景渐变色
    COLORREF color2;
    
    COLORREF color_text;//文字颜色
}RHeadStyle,*pHeadStyle;

typedef struct __RNHDR_GRID_ {
#define CTRLID_HEAD 0x0199
#define GRID_UPDATE 0x0001
    NMHDR hdr;
    int index;
    int cx1;
    int cx2;
}NHDRGrid,*LPNHDRGrid;

int Head_InitialSettings(HWND hwnd);
pHeadStyle Head_GetSettings(HWND hwnd);
int Head_ClearSettings(HWND hwnd);

LRESULT CALLBACK HeadOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
int Head_HitTest(HWND hwnd,LPHDHITTESTINFO phit);
int Head_SetCursor(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Head_StartAdjustItem(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Head_AdjustintItem(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Head_AdjustItemDone(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Head_Draw(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Head_Size(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Head_GetTotalSize(HWND hwnd);
