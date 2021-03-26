#include "Head.h"

#pragma comment(lib,"head.lib")

typedef struct _RSTRUCT_TREEGRID_ {
#define YCL_TREEGRID "YTreeGrid"
#define WM_TREEVIEWCUSTOMDRAW (UINT)(WM_USER+0x0310)
#define ID_TREEVIEW_01 0x0001
#define ID_GRIDHEADER_01 0x0002

    HWND head;
    HWND tree;
}RTreeGrid,*pTreeGrid;
LRESULT CALLBACK TreeGridOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
int TreeGrid_CtrlCreate(HWND treeGrid);
int TreeGrid_Clear(HWND treegrid);
int TreeGrid_InitialSettings(HWND hwnd);
pTreeGrid TreeGrid_GetSettings(HWND hwnd);
int TreeGrid_ClearSettings(HWND hwnd);

typedef enum FRAME_AREA_TYPE {
#define SCROLLBAR_PIXLS 13
#define THUMB_MIN_PIXLS 50
    ZCLOSE,
    ZMAX,
    ZMIN,
    ZICON,
    ZMENUBAR,
    ZCAPTION,
    ZTEXTTITLE,
    ZCONFIG,
    ZMSG,
    ZHSCROLL,
    ZHSTHUMB,
    ZVSCROLL,
    ZVSTHUMB,
    ZNOCARE
} EFNCZone;

typedef struct _RSTRUCT_TREEINFO_ {
    HDC memdc;
    HBITMAP bmp;
    HDC hdc;
    
    WNDPROC proc;
    WNDPROC pre_proc;
    
    POINT drag_pt;
    POINT drag_pos;
    POINT view_offset;
    
    HTREEITEM hover_item;
    HTREEITEM check_item;
}RTreeStyle,*pTreeStyle;
int Tree_InitialSettings(HWND hwnd);
pTreeStyle Tree_GetSettings(HWND hwnd);
int Tree_ClearSettings(HWND hwnd);
int Tree_Draw(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Tree_DrawItem(HWND hwnd,HDC hdc,RECT rc,int step_level,LPTVITEM pti);
int Tree_NCCalcSize(HWND hwnd,WPARAM wParam,LPARAM lParam);
int Tree_GetZoneRect(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL isallign_top_left);
int Tree_NCDrawScrollBar(HWND hwnd,UINT scrolltype);
LRESULT CALLBACK TreeOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
