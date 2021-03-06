#include <vcl.h>
#include <Winspool.h>
#include <Windows.h>
#include "Size_frm.h"
#include "Main_frm.h"
#include "Lupa_frm.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "Desk_frm.h"
//---------------------------------------------------------------------------
//#pragma link "..\\..\\Classes\\TSoft_DeskSwitcher.obj"
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
TDesk_form *Desk_form;
//---------------------------------------------------------------------------
__fastcall TDesk_form::TDesk_form(TComponent* Owner)
		  : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Align(void)
{
::GetWindowRect(Desktop->Screen->Hwnd,&Desktop->Screen->Rect);
::GetWindowRect(Handle,&options.rect);

if (options.rect.left	 < Desktop->Screen->Rect.left)
   {options.rect.left   = Desktop->Screen->Rect.left;
   }
if (options.rect.right  >= Desktop->Screen->Rect.right)
   {options.rect.left   = Desktop->Screen->Rect.right  - (options.rect.right-options.rect.left);
   }
if (options.rect.top	 < Desktop->Screen->Rect.top)
   {options.rect.top    = Desktop->Screen->Rect.top;
   }
if (options.rect.bottom >= Desktop->Screen->Rect.bottom)
   {options.rect.top    = Desktop->Screen->Rect.bottom - (options.rect.bottom-options.rect.top);
   }
SetWindowPos(this->Handle,NULL,
		options.rect.left,options.rect.top,0,0,
		SWP_NOCOPYBITS|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Resize()
{
options.rect.right  = options.rect.left + options.clientrect.right;
options.rect.bottom = options.rect.top  + options.clientrect.bottom;
AdjustWindowRectEx(&options.rect,
		GetWindowLong(this->Handle,GWL_STYLE),false,
		GetWindowLong(this->Handle,GWL_EXSTYLE));
SetWindowPos(this->Handle,NULL,
		0,0,(options.rect.right-options.rect.left),(options.rect.bottom-options.rect.top),
		SWP_NOCOPYBITS|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
tform_Calculate_Rect(); tform_Align(); tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Move()
{
SetWindowPos(this->Handle,NULL,
		options.rect.left,options.rect.top,0,0,
		SWP_NOCOPYBITS|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
tform_Align();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Redraw()
{
static bool lock = 0;
if (lock==1)
        return;
lock = 1;

static HBRUSH btnbrush = GetSysColorBrush(COLOR_BTNFACE);
static HBRUSH redbrush = CreateSolidBrush(RGB(255,115,115)), greenbrush = CreateSolidBrush(RGB(115,255,115)), bluebrush = CreateSolidBrush(RGB(115,115,255));
HBRUSH brush;

tform_Calculate_Rect();

SetStretchBltMode(Desktop->Screen->Context->Hdc,COLORONCOLOR);
Desktop->Screen->Context->Resize(options.clientrect.right,options.clientrect.bottom);
::FillRect(Desktop->Screen->Context->Hdc,&options.clientrect,btnbrush);
SetStretchBltMode(Desktop->Screen->Context->Hdc,HALFTONE);

for (int i = 1, x; i <= 4; i++)
    {
    if (Desktop->Virtual[i]->Printed==true) x = i;
	else x = 0;
    StretchBlt(Desktop->Screen->Context->Hdc,
    		 options.deskrect[i].left, options.deskrect[i].top, options.deskrect[i].right-options.deskrect[i].left, options.deskrect[i].bottom-options.deskrect[i].top,
    		 Desktop->Virtual[x]->Context->Hdc,
    		 0,0,Desktop->Virtual[x]->Rect.right,Desktop->Virtual[x]->Rect.bottom,
    		 SRCCOPY);
    }
for (int color, d = Desktop->Previous_Desktop_Index();;)
    {
     if (d==Desktop->Previous_Desktop_Index()) brush = bluebrush;
     else brush = redbrush;

     RECT temprect = options.deskrect[d];
     temprect.left--;
     temprect.top--;
     temprect.right++;
     temprect.bottom++;
     ::FrameRect(Desktop->Screen->Context->Hdc,&temprect,brush);
     temprect.left--;
     temprect.top--;
     temprect.right++;
     temprect.bottom++;
     ::FrameRect(Desktop->Screen->Context->Hdc,&temprect,brush);

     if (d!=Desktop->Active_Desktop_Index()) d =Desktop->Active_Desktop_Index();
     else break;
    }
tform_Select();

BitBlt(this->Canvas->Handle,
     	 0,0,options.clientrect.right,options.clientrect.bottom,
     	 Desktop->Screen->Context->Hdc,
     	 0,0,
     	 SRCCOPY);
lock = 0;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Select()
{
POINT cursorpoint;
::GetCursorPos(&cursorpoint);
::ScreenToClient(this->Handle,&cursorpoint);
static int  olddesk = 0, curdesk = 0, d;
static HWND oldhwnd = NULL, curhwnd = NULL;
static HBRUSH greenbrush = CreateSolidBrush(RGB(115,255,115));
HBRUSH brush;

for (d = 1; d <= 4; d++)
    {if (cursorpoint.x >= options.deskrect[d].left && cursorpoint.x < options.deskrect[d].right &&
	 cursorpoint.y >= options.deskrect[d].top  && cursorpoint.y < options.deskrect[d].bottom)
	{curdesk = d;
	 break;
	}
    }
if (curdesk!=0)
   {curhwnd = Desktop->Virtual[Desktop->Active_Desktop_Index()]->Handle_at_XY(
                (Desktop->Screen->Rect.right  * (cursorpoint.x-options.deskrect[curdesk].left)) / (options.deskrect[curdesk].right  - options.deskrect[curdesk].left),
                (Desktop->Screen->Rect.bottom * (cursorpoint.y-options.deskrect[curdesk].top))  / (options.deskrect[curdesk].bottom - options.deskrect[curdesk].top)
                );
   }
if (curdesk!=0 && curhwnd!=NULL)
   {
	 RECT temprect;
	 GetWindowRect(curhwnd,&temprect);

	 temprect.left = this->options.deskrect[curdesk].left + (temprect.left   * (this->options.deskrect[curdesk].right  - this->options.deskrect[curdesk].left)) / Desktop->Screen->Rect.right;
	 if (temprect.left < this->options.deskrect[curdesk].left)
	     temprect.left = this->options.deskrect[curdesk].left;
	     temprect.right = this->options.deskrect[curdesk].left + (temprect.right  * (this->options.deskrect[curdesk].right  - this->options.deskrect[curdesk].left)) / Desktop->Screen->Rect.right;
	 if (temprect.right > this->options.deskrect[curdesk].right)
	     temprect.right = this->options.deskrect[curdesk].right;
             temprect.top = this->options.deskrect[curdesk].top  + (temprect.top    * (this->options.deskrect[curdesk].bottom - this->options.deskrect[curdesk].top))  / Desktop->Screen->Rect.bottom;
	 if (temprect.top < this->options.deskrect[curdesk].top)
	     temprect.top = this->options.deskrect[curdesk].top;
             temprect.bottom = this->options.deskrect[curdesk].top  + (temprect.bottom * (this->options.deskrect[curdesk].bottom - this->options.deskrect[curdesk].top))  / Desktop->Screen->Rect.bottom;
	 if (temprect.bottom > this->options.deskrect[curdesk].bottom)
	     temprect.bottom = this->options.deskrect[curdesk].bottom;

	 InvertRect(Desktop->Screen->Context->Hdc,&this->options.deskrect[curdesk]);
	 InvertRect(Desktop->Screen->Context->Hdc,&temprect);

	  brush = greenbrush;

	 temprect.left--;
	 temprect.top--;
	 temprect.right++;
         temprect.bottom++;
	 ::FrameRect(Desktop->Screen->Context->Hdc,&temprect,brush);
	 temprect.left--;
	 temprect.top--;
	 temprect.right++;
         temprect.bottom++;
	  ::FrameRect(Desktop->Screen->Context->Hdc,&temprect,brush);

	 char text[64];
	 GetWindowText(curhwnd,text,63);
	 if (GetWindowTextLength(curhwnd)>0)
	    {this->Caption = "Okno: " + (String)text;
	    }
	 else
	    {this->Caption = "Okno: NULL";
	    }
	 olddesk = curdesk; oldhwnd = curhwnd;
   }
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Calculate_Rect()
{
  options.deskrect[0].left = 4;
 options.deskrect[0].right = options.clientrect.right   - 4;
   options.deskrect[0].top = 4;
options.deskrect[0].bottom = options.clientrect.bottom  - 4;

  options.deskrect[1].left = 4;
 options.deskrect[1].right = options.deskrect[1].left + (options.clientrect.right  - 16)/4;
   options.deskrect[1].top = 4;
options.deskrect[1].bottom = options.deskrect[1].top + options.clientrect.bottom - 4;

  options.deskrect[2].left = options.deskrect[1].right  + 4;
 options.deskrect[2].right = options.deskrect[2].left + (options.clientrect.right  - 16)/4;
   options.deskrect[2].top = options.deskrect[1].top;
options.deskrect[2].bottom = options.deskrect[1].bottom;

  options.deskrect[3].left = options.deskrect[2].right  + 4;
 options.deskrect[3].right = options.deskrect[3].left + (options.clientrect.right  - 16)/4;
   options.deskrect[3].top = options.deskrect[1].top;
options.deskrect[3].bottom = options.deskrect[1].bottom;

  options.deskrect[4].left = options.deskrect[3].right  + 4;
 options.deskrect[4].right = options.deskrect[4].left + (options.clientrect.right  - 16)/4;
   options.deskrect[4].top = options.deskrect[1].top;
options.deskrect[4].bottom = options.deskrect[1].bottom;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Initialize(void)
{
tform_Load();
Timer1->Interval = options.interval;
MenuItemTop->Checked = (options.zorder==(long)HWND_TOPMOST);
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
tform_Move(); tform_Resize();
SetClassLong(this->Handle,GCL_STYLE,GetClassLong(this->Handle,GCL_STYLE) | CS_SAVEBITS);
if (options.zoomed)
	SetWindowLong(this->Handle,GWL_STYLE,GetWindowLong(this->Handle,GWL_STYLE) | WS_MAXIMIZE);
if (options.visible)
	this->Show();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Load(void)
{
KluczRejestuSystemuWindows->OpenKey("Software\\TSoft_\\Panel\\Virtual",true);

if (KluczRejestuSystemuWindows->ValueExists("clientrect"))
   {KluczRejestuSystemuWindows->ReadBinaryData("clientrect",&options.clientrect, sizeof(RECT));
   }
else
   {options.clientrect.right = 4*96+12;
    options.clientrect.bottom = 1*72+12;
   }
if (KluczRejestuSystemuWindows->ValueExists("rect"))
   {KluczRejestuSystemuWindows->ReadBinaryData("rect",&options.rect, sizeof(RECT));
   }
else
   {options.rect.left = 0;
    options.rect.top = 0;
   }
if (KluczRejestuSystemuWindows->ValueExists("zoomed"))
   {options.zoomed = KluczRejestuSystemuWindows->ReadBool("zoomed");
   }
else
   {options.zoomed = false;
   }
if (KluczRejestuSystemuWindows->ValueExists("alpha"))
   {options.alpha = KluczRejestuSystemuWindows->ReadInteger("alpha");
   }
else
   {options.alpha = -1;
   }
if (KluczRejestuSystemuWindows->ValueExists("clickthrough"))
   {options.clickthrough  = KluczRejestuSystemuWindows->ReadInteger("clickthrough");
   }
else
   {options.clickthrough  = 0;
   }
if (KluczRejestuSystemuWindows->ValueExists("visible"))
   {options.visible = KluczRejestuSystemuWindows->ReadBool("visible");
   }
else
   {options.visible = false;
   }
if (KluczRejestuSystemuWindows->ValueExists("interval"))
   {options.interval = KluczRejestuSystemuWindows->ReadInteger("interval");
   }
else
   {options.interval = 30000;
   }
if (KluczRejestuSystemuWindows->ValueExists("zorder"))
   {options.zorder = KluczRejestuSystemuWindows->ReadInteger("zorder");
   }
else
   {options.zorder = (long)HWND_TOPMOST;
   }
KluczRejestuSystemuWindows->CloseKey();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::tform_Save(void)
{
if (!options.zoomed)
   {::GetClientRect(this->Handle,&options.clientrect);
    ::GetWindowRect(this->Handle,&options.rect);
   }
KluczRejestuSystemuWindows->OpenKey("Software\\TSoft_\\Panel\\Virtual",true);

KluczRejestuSystemuWindows->WriteBinaryData("rect",&options.rect,sizeof(RECT));
KluczRejestuSystemuWindows->WriteBinaryData("clientrect",&options.clientrect,sizeof(RECT));
KluczRejestuSystemuWindows->WriteBool("zoomed",options.zoomed);
KluczRejestuSystemuWindows->WriteInteger("alpha",options.alpha);
KluczRejestuSystemuWindows->WriteInteger("clickthrough",options.clickthrough);
KluczRejestuSystemuWindows->WriteBool("visible",options.visible);
KluczRejestuSystemuWindows->WriteInteger("interval", options.interval);
KluczRejestuSystemuWindows->WriteInteger("zorder",options.zorder);

KluczRejestuSystemuWindows->CloseKey();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormCreate(TObject *Sender)
{
hInst = (HINSTANCE)HInstance;
::SetStretchBltMode(this->Canvas->Handle, STRETCH_DELETESCANS);
Ruszacz = new ts::WindowsMover();
KluczRejestuSystemuWindows = new TRegistry;
updateing = 0;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormShow(TObject *Sender)
{
Timer1->Enabled = true;
if (Main_form!=NULL ? Main_form->SpeedButtonDeskGrid!=NULL : false)
    Main_form->SpeedButtonDeskGrid->Down = true;
MenuItemZwin->Caption = "&Hide";
options.visible = true;
tform_Align();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormHide(TObject *Sender)
{
Timer1->Enabled = false;
if (Main_form!=NULL ? Main_form->SpeedButtonDeskGrid!=NULL : false)
    Main_form->SpeedButtonDeskGrid->Down = false;
ShowWindow(Application->Handle,SW_SHOWNA);
ShowWindow(Application->Handle,SW_HIDE);
MenuItemZwin->Caption = "&Show";
options.visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormDestroy(TObject *Sender)
{
delete Ruszacz;
delete KluczRejestuSystemuWindows;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormResize(TObject *Sender)
{
options.zoomed = IsZoomed(Handle);
tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormPaint(TObject *Sender)
{
if (updateing) return;
updateing = true;
SetWindowPos(Handle,
	     (void*)options.zorder,
	     0,0,0,0,
	     SWP_NOCOPYBITS|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING|SWP_NOREDRAW);
RECT temprect;
::GetClientRect(this->Handle,&temprect);
if (temprect.right!=Desktop->Screen->Context->Bitmap->Info->bmiHeader.biWidth || temprect.bottom!=Desktop->Screen->Context->Bitmap->Info->bmiHeader.biHeight)
   {tform_Redraw();
   }
else
   {::GetClipBox(this->Canvas->Handle,&temprect);
    ::BitBlt(this->Canvas->Handle,
		 temprect.left,temprect.top,temprect.right-temprect.left,temprect.bottom-temprect.top,
		 Desktop->Screen->Context->Hdc,
		 temprect.left,temprect.top,
		 SRCCOPY);
   }
updateing = 0;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormMouseDown(TObject *Sender,
		TMouseButton Button, TShiftState Shift, int X, int Y)
{
if (Button==mbRight)
   {updateing = true;
    POINT point;
    GetCursorPos(&point);
    DeskPopupMenu->Popup(point.x,point.y);
    updateing = 0;
    return;
   }
if (Button==mbLeft)
   {for (int i = 4; i > 0; i--)
	{if (X >= options.deskrect[i].left && X < options.deskrect[i].right && Y >= options.deskrect[i].top && Y < options.deskrect[i].bottom)
	    {Desktop_Switch(i,true);
	     break;
            }
	}
   }
if (Button==mbMiddle)
   {tform_Redraw();
   }
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormMouseMove(TObject *Sender,
		TShiftState Shift, int X, int Y)
{
//
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormMouseUp(TObject *Sender,
		TMouseButton Button, TShiftState Shift, int X, int Y)
{
/*if (Button!=mbMiddle)
	 return;
	for (int i = 4; i > 0; i--)
	{if (X >= options.deskrect[i].left && X < options.deskrect[i].right && Y >= options.deskrect[i].top && Y < options.deskrect[i].bottom)
		  {if (Desktop->Active_Desktop_Index()==i) Desktop_Switch(Desktop->Previous_Desktop_Index(),true);
			break;
		  }
	 }
*/
}
//---------------------------------------------------------------------------

int __fastcall TDesk_form::Desktop_Switch(int newDesk, bool redraw)
{
Desktop->Activate(newDesk);

switch (newDesk)
{
case 0: Main_form->SpeedButtonDesk5->Down = true;
break;
case 1: Main_form->SpeedButtonDesk1->Down = true;
break;
case 2: Main_form->SpeedButtonDesk2->Down = true;
break;
case 3: Main_form->SpeedButtonDesk3->Down = true;
break;
case 4: Main_form->SpeedButtonDesk4->Down = true;
break;
}
if (redraw) tform_Redraw();
return newDesk;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::FormKeyDown(TObject *Sender, WORD &Key,
		TShiftState Shift)
{
switch (Key) {
case '1':
	Desktop_Switch(1,true);
break;
case '2':
	Desktop_Switch(2,true);
break;
case '3':
	Desktop_Switch(3,true);
break;
case '4':
	Desktop_Switch(4,true);
break;
case '0':
	Desktop_Switch(0,true);
break;

case VK_ESCAPE: Hide();

break;
}
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::Timer1Timer(TObject *Sender)
{
if (updateing)
    return;
updateing = true;
Desktop->Collect(); Desktop->Capture();tform_Redraw();
updateing = 0;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::Timer2Timer(TObject *Sender)
{
//if (updateing)
//    return;
//updateing = true;
if ((Lupa_form->Left+Lupa_form->Width < Desk_form->Left || Lupa_form->Top+Lupa_form->Height < Desk_form->Top) ||
    (Lupa_form->Left > Desk_form->Left + Desk_form->Width || Lupa_form->Top > Desk_form->Top + Desk_form->Height))
SetWindowPos(Handle,(void*)options.zorder,
     		 0,0,0,0,
     		 SWP_NOCOPYBITS|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOSENDCHANGING);
//updateing = 0;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::SubMenuPrzeswitClick(TObject *Sender)
{
if (options.clickthrough==true)
	 MenuItemTrans->Checked = true;

if (options.alpha==0)
	NOFF->Checked = true;
else
if (options.alpha==20)
	N201->Checked = true;
else
if (options.alpha==30)
	N301->Checked = true;
else
if (options.alpha==40)
	N401->Checked = true;
else
if (options.alpha==50)
	N501->Checked = true;
else
if (options.alpha==60)
	N601->Checked = true;
else
if (options.alpha==70)
	N701->Checked = true;
else
if (options.alpha==80)
	N801->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemTransClick(TObject *Sender)
{
options.clickthrough = !options.clickthrough;
MenuItemTrans->Checked = options.clickthrough;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::NOFFClick(TObject *Sender)
{
options.alpha = -1;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
NOFF->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N101Click(TObject *Sender)
{
options.alpha = 10;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N201->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N201Click(TObject *Sender)
{
options.alpha = 20;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N201->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N301Click(TObject *Sender)
{
options.alpha = 30;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N301->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N401Click(TObject *Sender)
{
options.alpha = 40;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N401->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N501Click(TObject *Sender)
{
options.alpha = 50;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N201->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N601Click(TObject *Sender)
{
options.alpha = 60;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N601->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N701Click(TObject *Sender)
{
options.alpha = 70;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N701->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::N801Click(TObject *Sender)
{
options.alpha = 80;
Desktop->Action(SET_TRANSPARENCY,this->Handle,options.alpha,options.clickthrough);
N801->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::SubMenuSizeClick(TObject *Sender)
{
if (options.clientrect.right==4*384+12 && options.clientrect.bottom==1*256+12)
   {MenuItemSize384->Checked  = true;
   }
else
if (options.clientrect.right==4*256+12 && options.clientrect.bottom==1*192+12)
   {MenuItemSize256->Checked  = true;
   }
else
if (options.clientrect.right==4*192+12 && options.clientrect.bottom==1*144+12)
   {MenuItemSize192->Checked  = true;
   }
else
if (options.clientrect.right==4*144+12 && options.clientrect.bottom==1*108+12)
   {MenuItemSize144->Checked  = true;
   }
else
if (options.clientrect.right==4*96+12 && options.clientrect.bottom==1*72+12)
   {MenuItemSize96->Checked   = true;
   }
else
if (options.clientrect.right==4*64+12 && options.clientrect.bottom==1*48+12)
   {MenuItemSize64->Checked   = true;
   }
else
if (options.clientrect.right==4*48+12 && options.clientrect.bottom==1*36+12)
   {MenuItemSize48->Checked   = true;
   }
else
if (options.clientrect.right==4*32+12 && options.clientrect.bottom==1*24+12)
   {MenuItemSize32->Checked   = true;
   }
else
MenuItemSizeAdvanced->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSizeAdvancedClick(TObject *Sender)
{
Size_form->TrackBar1->Position = options.clientrect.right;
Size_form->TrackBar2->Position = options.clientrect.bottom;
Size_form->CheckBox1->Checked  = options.clientrect.bottom*4==options.clientrect.right*3;
long  acc = options.zorder;
	    options.zorder = (long)HWND_BOTTOM;
if (Size_form->Execute(this)==mrOk)
   {options.clientrect.right = Size_form->TrackBar1->Position; options.rect.bottom = Size_form->TrackBar2->Position;
    tform_Resize();
   }
options.zorder = acc;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize32Click(TObject *Sender)
{
options.clientrect.right = 4*24+16; options.clientrect.bottom = 24+8;
tform_Resize();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize48Click(TObject *Sender)
{
options.clientrect.right = 4*48+12; options.clientrect.bottom = 48+8;
tform_Resize();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize64Click(TObject *Sender)
{
options.clientrect.right = 4*64+12; options.clientrect.bottom = 64+8;
tform_Resize();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize96Click(TObject *Sender)
{
options.clientrect.right = 4*96+12; options.clientrect.bottom = 96+8;
tform_Resize();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize144Click(TObject *Sender)
{
options.clientrect.right = 4*144+12; options.clientrect.bottom = 144+8;
tform_Resize();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize192Click(TObject *Sender)
{
options.clientrect.right = 4*192+12; options.clientrect.bottom = 192+8;
tform_Resize();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize256Click(TObject *Sender)
{
options.clientrect.right = 4*256+12; options.clientrect.bottom = 256+8;
tform_Resize();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemSize384Click(TObject *Sender)
{
options.clientrect.right = 4*384+12; options.clientrect.bottom = 384+8;
tform_Resize();
}
//---------------------------------------------------------------------------
void __fastcall TDesk_form::MenuItemBiurko0Click(TObject *Sender)
{
Desktop->Move(Desktop->Active_Desktop_Index(), 0);
Desktop->Capture();
tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemBiurko1Click(TObject *Sender)
{
Desktop->Move(Desktop->Active_Desktop_Index(), 1);
Desktop->Capture();
tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemBiurko2Click(TObject *Sender)
{
Desktop->Move(Desktop->Active_Desktop_Index(), 2);
Desktop->Capture();
tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemBiurko3Click(TObject *Sender)
{
Desktop->Move(Desktop->Active_Desktop_Index(), 3);
Desktop->Capture();
tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemBiurko4Click(TObject *Sender)
{
Desktop->Move(Desktop->Active_Desktop_Index(), 4);
Desktop->Capture();
tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemZbierzClick(TObject *Sender)
{
for (int d = 1; d < 5; d++)
	{if (d!=Desktop->Active_Desktop_Index())
			Desktop->Move(d,Desktop->Active_Desktop_Index());
	}
Desktop->Capture();
tform_Redraw();
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemPrzeniesClick(TObject *Sender)
{
MenuItemBiurko1->Enabled = Desktop->Active_Desktop_Index()!=1;
MenuItemBiurko2->Enabled = Desktop->Active_Desktop_Index()!=2;
MenuItemBiurko3->Enabled = Desktop->Active_Desktop_Index()!=3;
MenuItemBiurko4->Enabled = Desktop->Active_Desktop_Index()!=4;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::SubMenuOknaClick(TObject *Sender)
{
MenuItemBiurko0->Visible = Desktop->Active_Desktop_Index()!=0;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemZwinClick(TObject *Sender)
{
this->Visible=!this->Visible;
//SetForegroundWindow(Desk_form->Handle);
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::MenuItemTopClick(TObject *Sender)
{
    options.zorder^=(long)HWND_TOPMOST;
if (options.zorder==(long)HWND_TOPMOST) MenuItemTop->Checked = true;
else
   {MenuItemTop->Checked = false;
   }
SetWindowPos(Handle,(void*)options.zorder,0,0,0,0,SWP_NOCOPYBITS|SWP_NOSIZE|SWP_NOMOVE);
}
//---------------------------------------------------------------------------

/*
void __fastcall TDesk_form::SubMenuTimer1Click(TObject *Sender)
{
if (options.interval==10000)
	U010->Checked = true;
else
if (options.interval==20000)
	U020->Checked = true;
else
if (options.interval==30000)
	U030->Checked = true;
else
if (options.interval==60000)
	U060->Checked = true;
else
if (options.interval==120000)
	U120->Checked = true;
else
if (options.interval==3000000)
	U180->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::U010Click(TObject *Sender)
{
options.interval = 10000;
Timer1->Interval = options.interval;
U010->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::U020Click(TObject *Sender)
{
options.interval = 20000;
Timer1->Interval = options.interval;
U020->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::U030Click(TObject *Sender)
{
options.interval = 30000;
Timer1->Interval = options.interval;
U030->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::U060Click(TObject *Sender)
{
options.interval = 60000;
Timer1->Interval = options.interval;
U060->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::U120Click(TObject *Sender)
{
options.interval = 120000;
Timer1->Interval = options.interval;
U120->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TDesk_form::U180Click(TObject *Sender)
{
options.interval = 180000;
Timer1->Interval = options.interval;
U180->Checked = true;
}
//---------------------------------------------------------------------------
*/



void __fastcall TDesk_form::FormActivate(TObject *Sender)
{
::GetWindowRect(Desktop->Screen->Hwnd,&Desktop->Screen->Rect);
}
//---------------------------------------------------------------------------

