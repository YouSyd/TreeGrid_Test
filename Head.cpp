#include "head.h"

int Head_InitialSettings(HWND hwnd)
{
    pHeadStyle hs=(pHeadStyle)calloc(sizeof(RHeadStyle),1);
    if(!hs) return -1;
        
    hs->color1=RGB(15,15,15);
    hs->color2=RGB(30,30,30);
    hs->color_text=RGB(0,128,250);
    
    hs->proc=HeadOwnProc;
    hs->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)hs->proc);
    
    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)hs);
    return 0;
}

pHeadStyle Head_GetSettings(HWND hwnd)
{
    return (pHeadStyle)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int Head_ClearSettings(HWND hwnd)
{
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return -1;
        
    SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)hs->pre_proc);
    SetWindowLongPtr(hwnd,GWLP_USERDATA,NULL);
    
    free(hs);
    return 0;
}

int Head_HitTest(HWND hwnd,LPHDHITTESTINFO phit)
{    
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return -1;
    
    int item_count=Header_GetItemCount(hwnd);
    int total_offset=0;
    
    for(int index=0;index<item_count;index++) {
        HDITEM hdi={0};
        
        hdi.mask=HDI_WIDTH;
        Header_GetItem(hwnd,index,&hdi);
        
        total_offset+=hdi.cxy;
        if(abs((total_offset+hs->offset)-phit->pt.x)<5) {
            phit->flags=HHT_ONDIVIDER;
            phit->iItem=index;
            return index;
        }
        else if((total_offset+hs->offset)-phit->pt.x>0) {
            phit->flags=HHT_ONHEADER;
            phit->iItem=index;
            return index;
        }
    }
    phit->flags=HHT_NOWHERE;
    phit->iItem=-1;
    return phit->iItem;
}

int Head_SetCursor(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd,&pt);
    
    HDHITTESTINFO hit={0};
    memcpy(&hit.pt,&pt,sizeof(POINT));
    SendMessage(hwnd,HDM_HITTEST,0,(LPARAM)&hit);
    
    if(hit.flags==HHT_ONDIVIDER) SetCursor(LoadCursor(NULL,IDC_SIZEWE));
    else if(hit.flags==HHT_ONHEADER) SetCursor(LoadCursor(NULL,IDC_ARROW));
        
    return 1;
}

int Head_StartAdjustItem(HWND hwnd,WPARAM wParam,LPARAM lParam)
{    
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return -1;
    
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    HDHITTESTINFO hit={0};
    
    memcpy(&hit.pt,&pt,sizeof(POINT));
    SendMessage(hwnd,HDM_HITTEST,0,(LPARAM)&hit);
        
    if(hit.flags==HHT_ONDIVIDER) {
        SetCapture(hwnd);
        hs->dragging=TRUE;
        hs->dragpt=pt.x;
        hs->dragindex=hit.iItem;
        SetCursor(LoadCursor(NULL,IDC_SIZEWE));
    }
    
    RECT rc_item;
    Header_GetItemRect(hwnd,hit.iItem,&rc_item);
    OffsetRect(&rc_item,hs->offset,0);
    InvalidateRect(hwnd,&rc_item,TRUE);
    return 1;
}

int Head_AdjustintItem(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return -1;
    
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    
    if(GetCapture()==hwnd&&hs->dragging) {
        hs->dragnow=pt.x;
        int item_count=Header_GetItemCount(hwnd);
        for(int index=hs->dragindex;index<item_count;index++) {        
            RECT rcItem;
            Header_GetItemRect(hwnd,index,&rcItem);
            OffsetRect(&rcItem,hs->offset,0);
            if(index==hs->dragindex&&pt.x<rcItem.left+5) hs->dragnow=rcItem.left+5;
            InvalidateRect(hwnd,&rcItem,(index==item_count-1)?TRUE:FALSE);
        }
    }
    return 0; 
}

int Head_AdjustItemDone(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return -1;
    
    POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
    if(hs->dragging&&GetCapture()==hwnd) {
        HDITEM hdi={0};
        
        hdi.mask=HDI_WIDTH;
        Header_GetItem(hwnd,hs->dragindex,&hdi);
        hdi.cxy+=(pt.x-hs->dragpt);
        if(hdi.cxy<=0) hdi.cxy=5;
        Header_SetItem(hwnd,hs->dragindex,&hdi);
        ReleaseCapture();
        hs->dragging=FALSE;
        hs->dragnow=-1;
        SetCursor(LoadCursor(NULL,IDC_ARROW));
        
        //修正offset.
        RECT rc;
        GetClientRect(hwnd,&rc);
        SCROLLINFO si;
        si.nMax=Head_GetTotalSize(hwnd);
        int cur_pos;
        if(rc.right-rc.left>si.nMax) hs->offset=0;
        else if((-hs->offset)+(rc.right-rc.left)>si.nMax) {
            hs->offset=-(si.nMax-(rc.right-rc.left));
        }
        InvalidateRect(hwnd,NULL,TRUE);
        
        {
            //通知父窗口有更新
            NHDRGrid grid;
            grid.hdr.hwndFrom=hwnd;
            grid.hdr.idFrom=CTRLID_HEAD;
            grid.hdr.code=GRID_UPDATE;
            grid.index=-1;
            grid.cx1=rc.left;
            grid.cx2=rc.right;
            SendMessage(GetParent(hwnd),WM_NOTIFY,(WPARAM)hwnd,(LPARAM)&grid);
        }   
    }
    return 0;
}

int Head_GetTotalSize(HWND hwnd)
{
    int item_count=Header_GetItemCount(hwnd);
    int total_offset=0;
    for(int index=0;index<item_count;index++) {
        HDITEM hdi={0};
        
        hdi.mask=HDI_WIDTH;
        Header_GetItem(hwnd,index,&hdi);
        
        total_offset+=hdi.cxy;
    }
    return total_offset;
}

int Head_Draw(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return -1;
        
    PAINTSTRUCT ps;
    BeginPaint(hwnd,&ps);
    
    HDC hdc=ps.hdc;
    int cx=ps.rcPaint.right-ps.rcPaint.left;
    int cy=ps.rcPaint.bottom-ps.rcPaint.top;
    
    RECT rc;
    GetClientRect(hwnd,&rc);
    
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
    HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    HFONT pre_font=(HFONT)SelectObject(memdc,(HFONT)SendMessage(hwnd,WM_GETFONT,0,0));
    
    RECT rc_bmp;
    CopyRect(&rc_bmp,&rc);
    InflateRect(&rc_bmp,1,1);
    
    //RECT rhd_1,rhd_2;
    //CopyRect(&rhd_1,&rc_bmp);
    //CopyRect(&rhd_2,&rc_bmp);
    //rhd_1.bottom=(rhd_1.bottom+rhd_1.top)/2;
    //rhd_2.top=rhd_1.bottom;
        
    GradientRect(memdc,rc_bmp,hs->color1,hs->color2,1);
    //GradientRect(memdc,rhd_2,hs->color2,hs->color1,1);
    
    SetTextColor(memdc,hs->color_text);
    SetBkMode(memdc,TRANSPARENT);
    
    POINT pt_org;
    OffsetViewportOrgEx(memdc,-(ps.rcPaint.left)+hs->offset,-(ps.rcPaint.top),&pt_org);
    
    int total_offset=0;
    int col_count=Header_GetItemCount(hwnd);
    RECT rcItem;
    HDITEM item={0};
    char col_title[256]="";
    for(int index=0;index<col_count;index++) {
        Header_GetItemRect(hwnd,index,&rcItem);
        OffsetRect(&rcItem,hs->offset,0);
                
        if(rcItem.right<=ps.rcPaint.left) continue;        
        else if(rcItem.left<=ps.rcPaint.left&&rcItem.right>ps.rcPaint.left) {
            //需要部分绘制
            total_offset=rcItem.right-ps.rcPaint.left;
        }
        else if(rcItem.left>=ps.rcPaint.left&&total_offset<cx) {
            //需要整段绘制
            total_offset+=rcItem.right-rcItem.left;
        }
        else if(total_offset>=cx) break;
                
        //还原
        OffsetRect(&rcItem,-(hs->offset),0);
        item.mask=HDI_TEXT|HDI_STATE|HDI_WIDTH;
        item.pszText=col_title;
        item.cchTextMax=sizeof(col_title);
        Header_GetItem(hwnd,index,&item);
        DrawText(memdc,col_title,-1,&rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
               
        //绘制立体分割线
        POINT pt[7]={{rcItem.right-2,rcItem.top},
                     {rcItem.right-2,rcItem.bottom},
                     {rcItem.right-1,rcItem.top},
                     {rcItem.right-1,rcItem.bottom},
                     {hs->dragnow-hs->offset,rcItem.top},
                     {hs->dragnow-hs->offset,rcItem.bottom}};
        
        COLORREF color_1=RGB(10,10,10),color_2=RGB(40,40,40);
        HPEN sp_line1=CreatePen(PS_SOLID,1,color_1);
        HPEN sp_line2=CreatePen(PS_SOLID,1,color_2);
        HPEN pre_sp;
        
        pre_sp=(HPEN)SelectObject(memdc,sp_line1);
        MoveToEx(memdc,pt[0].x,pt[0].y,&(pt[6]));
        LineTo(memdc,pt[1].x,pt[1].y);
        
        SelectObject(memdc,sp_line2);
        MoveToEx(memdc,pt[2].x,pt[2].y,&(pt[6]));
        LineTo(memdc,pt[3].x,pt[3].y);

        if(GetCapture()==hwnd&&hs->dragging) {
            COLORREF color_drag=RGB(0,100,200);
            HPEN pen_drag=CreatePen(PS_SOLID,2,color_drag);
            SelectObject(memdc,pen_drag);
            MoveToEx(memdc,pt[4].x,pt[4].y,&(pt[6]));
            LineTo(memdc,pt[5].x,pt[5].y);
            DeleteObject(pen_drag);
        }     
        
        DeleteObject(sp_line1);
        DeleteObject(sp_line2); 
    }
    
    SetViewportOrgEx(memdc,pt_org.x,pt_org.y,NULL);
    
    BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,cx,cy,
           memdc,0,0,SRCCOPY);
           
    DeleteObject(SelectObject(memdc,pre_bmp));
    DeleteDC(memdc);
    
    EndPaint(hwnd,&ps);
    return 0;
}

int Head_Size(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return -1;
        
    int height=HIWORD(lParam);
    int width=LOWORD(lParam);
    int item_count=Header_GetItemCount(hwnd);
    int total_offset=0;
    for(int index=0;index<item_count;index++) {
        HDITEM hdi={0};
        
        hdi.mask=HDI_WIDTH;
        Header_GetItem(hwnd,index,&hdi);
        
        total_offset+=hdi.cxy;
    }
    
    //offset维护了已经滚动的距离（被隐藏的部分），左移为负，右移为正。
    if(hs->offset<0) {
        if(width>=total_offset) {
            ScrollWindow(hwnd,-hs->offset,0,NULL,NULL);
            hs->offset=0;
        }
        else if(abs(hs->offset)+width>=total_offset&&width<total_offset) {
            int offset_tmp=width-total_offset;
            ScrollWindow(hwnd,offset_tmp-hs->offset,0,NULL,NULL);
            hs->offset=offset_tmp;
        }
    }
    
    return 0;
}

LRESULT CALLBACK HeadOwnProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    pHeadStyle hs=Head_GetSettings(hwnd);
    if(!hs) return 0;
    
    switch(message) {
   	case HDM_LAYOUT: {
        LRESULT result=CallWindowProc(hs->pre_proc,hwnd, message, wParam, lParam);
    
        LPHDLAYOUT lphdLayout=(LPHDLAYOUT)lParam;
        lphdLayout->pwpos->cy=25;
        lphdLayout->prc->top=lphdLayout->pwpos->cy;
       		
        return result;
   	} break;
   	case HDM_HITTEST: {
   	    LPHDHITTESTINFO phit=(LPHDHITTESTINFO)lParam;
   	    return Head_HitTest(hwnd,phit);
   	} break;
   	case WM_HSCROLL: {
        UINT scroll_state=LOWORD(wParam);
        UINT pos=HIWORD(wParam);
        
        if(scroll_state!=SB_THUMBPOSITION) break;
        int offset=(int)pos-(int)abs(hs->offset);//左负 右正
        if(offset==0) break;
        
        RECT rc,rcScroll;
        GetClientRect(hwnd,&rc);
        CopyRect(&rcScroll,&rc);
        //滚动一部分
        //刷新一部分
        if(offset>0) {//右滚动 左移
            rc.left=rc.right-offset;
            rcScroll.right-=offset;
        }
        else {
            rc.right=-offset;
            rcScroll.left-=offset;
        }
        ScrollWindow(hwnd,-offset,0,NULL,&rcScroll);
        hs->offset=(-pos);
        InvalidateRect(hwnd,&rc,TRUE);
   	}
   	case WM_SETCURSOR: {
        UINT hit_pos=LOWORD(lParam);
        UINT trigger_msg=HIWORD(lParam);
        
        if(hit_pos==HTCLIENT&&trigger_msg==WM_MOUSEMOVE) return Head_SetCursor(hwnd,wParam,lParam);
   	} break;
   	case WM_MOUSEMOVE: {
  	    Head_AdjustintItem(hwnd,wParam,lParam);
   	} break;
   	case WM_LBUTTONDOWN: {
        return Head_StartAdjustItem(hwnd,wParam,lParam);
   	} break;
   	case WM_LBUTTONUP: {
        Head_AdjustItemDone(hwnd,wParam,lParam);
   	} break;
   	case WM_PAINT: {
        Head_Draw(hwnd,wParam,lParam);
        return 1;
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
    case WM_SIZE: {
        Head_Size(hwnd,wParam,lParam);
    } break; 
    case WM_NCDESTROY: {
        WNDPROC pre_proc=hs->pre_proc;
        Head_ClearSettings(hwnd);
        if(pre_proc) return CallWindowProc(hs->pre_proc,hwnd,message,wParam,lParam);
    } break;  
    }
    
    return CallWindowProc(hs->pre_proc,hwnd,message,wParam,lParam);    
}

void GradientRect(HDC hdc,RECT rc_tmp,COLORREF c1,COLORREF c2,int verical_or_horizen)
{
    RECT rc;
    CopyRect(&rc,&rc_tmp);
    
    Point pt[4]={Point(rc.left,rc.top),Point(rc.right,rc.top),Point(rc.right,rc.bottom),Point(rc.left,rc.bottom)};
    Rect rect(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
    Graphics graphic(hdc);
    GraphicsPath path;
    LinearGradientBrush brush(rect,Color(255,GetRValue(c1),GetGValue(c1),GetBValue(c1)),
                                   Color(255,GetRValue(c2),GetGValue(c2),GetBValue(c2)),
                              verical_or_horizen?LinearGradientModeVertical:LinearGradientModeHorizontal);
    
    graphic.SetSmoothingMode(SmoothingModeHighQuality);
    path.AddLines(pt,4);
    graphic.FillPath(&brush,&path);
}
