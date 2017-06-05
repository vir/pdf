#include "WxTabFrame.hpp"
#include "WxTabCanvas.hpp"
#include "WxTabDocument.hpp"

BEGIN_EVENT_TABLE(WxTabCanvas, MyCanvas)
	EVT_PAINT  (WxTabCanvas::OnPaint)
	EVT_MOTION (WxTabCanvas::OnMouse)
	EVT_LEFT_DOWN(WxTabCanvas::OnMouse)
	EVT_LEFT_UP(WxTabCanvas::OnMouse)
END_EVENT_TABLE()

WxTabCanvas::WxTabCanvas(WxTabFrame *parent)
	: MyCanvas(parent)
{
	SetFrame(parent);
	m_my_frame = parent;
}

#include "../wxview/PagePaintHelper.h"

void WxTabCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	PrepareDC(dc);
	m_my_frame->PrepareDC(dc);
	dc.Clear();

	if(theDocument) {
		PagePaintHelper m(dc, theDocument->Rotation(), theDocument->Scale()/100.0);
		theDocument->GetPageObject()->draw(&m);
		theDocument->DrawGrid(&dc);
	}
}

void WxTabCanvas::SetScrolledPageSize()
{
	if(! theDocument)
		return;
	SetPage(theDocument->GetPageObject());
}


