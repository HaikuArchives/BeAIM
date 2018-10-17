/* MakSplitterView
		-By Makhno
*/

// A few minor changes added by Greg Nichols, so it would better suit my
// purposes for BeAIM... very very nice overall, though!

#include "MakSplitterView.h"
#include <Application.h>
#include <Window.h>
#include <GraphicsDefs.h>

MakSplitterView::MakSplitterView (BRect pos,BView *lu,BView *rd,bool orientation,uint32 rezmode,rgb_color col)
				:BView(pos,NULL,rezmode,B_WILL_DRAW|B_FRAME_EVENTS)
{
	SetViewColor(col);
	mouseisdown=false;
	SetMouseEventMask(0);
	lui=lu;
	rdi=rd;
	h_or_v=orientation;
}

void MakSplitterView::MouseDown(BPoint where)
{
	mouseisdown=true;
	oldpoint=where;
	SetMouseEventMask(B_POINTER_EVENTS);
	Window()->PostMessage( new BMessage(MAK_START_SPLITTING) );
	if (h_or_v)
		be_app->SetCursor(&Mak_Hcursor);
	else
		be_app->SetCursor(&Mak_Vcursor);
}

void MakSplitterView::MouseUp(BPoint p)
{
	mouseisdown=false;
	SetMouseEventMask(0);
	be_app->SetCursor(B_HAND_CURSOR);
	Window()->PostMessage( new BMessage(MAK_DONE_SPLITTING) );
	MoveViews(p);
}

void MakSplitterView::MouseMoved(BPoint p,uint32 transit,const BMessage *msg)
{
	if(mouseisdown==true)
	{
		MoveViews(p);
	}
	else
	{
		if (transit==B_INSIDE_VIEW)
		{
			if (h_or_v)
				be_app->SetCursor(&Mak_Hcursor);
			else
				be_app->SetCursor(&Mak_Vcursor);
		}
		else
			be_app->SetCursor(B_HAND_CURSOR);
	}
}

void MakSplitterView::MoveViews(BPoint p)
{
	BPoint distance(0,0);

	if (h_or_v)
	{
		distance.x=p.x-oldpoint.x;
		if (lui->Frame().right+distance.x<lui->Frame().left)
			distance.x=lui->Frame().left-Frame().left;
		if (rdi->Frame().left+distance.x>rdi->Frame().right)
			distance.x=rdi->Frame().right-Frame().right;

		distance.x=distance.x/4;	//This stops flicker

		lui->ResizeBy(distance.x,0);
		rdi->MoveBy(distance.x,0);
		rdi->ResizeBy(-distance.x,0);
	}
	else
	{
		distance.y=p.y-oldpoint.y;
		if (lui->Frame().bottom+distance.y<lui->Frame().top)
			distance.y=lui->Frame().top-Frame().top;
		if (rdi->Frame().top+distance.y>rdi->Frame().bottom)
			distance.y=rdi->Frame().bottom-Frame().bottom;

		distance.y=distance.y/4;  	//This stops flicker

		lui->ResizeBy(0,distance.y);
		rdi->MoveBy(0,distance.y);
		rdi->ResizeBy(0,-distance.y);
	}
	MoveBy(distance.x,distance.y);
}

void MakSplitterView::Draw(BRect updateRect) {

	BRect frame = Bounds();

	// the highlights are hardcoded... gotta fix this
	SetHighColor( 130, 130, 130 );
	StrokeLine( frame.LeftTop(), frame.RightTop() );
	StrokeLine( frame.LeftBottom(), frame.RightBottom() );
	SetHighColor( 255, 255, 255 );
	frame.OffsetBy( 0, 1 );
	StrokeLine( frame.LeftTop(), frame.RightTop() );
}
