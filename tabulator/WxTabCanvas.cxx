#include "WxTabFrame.hpp"
#include "WxTabCanvas.hpp"
//#include <iomanip> /* for setiosflags and so */
//#include "../PDF.hpp"
#include "WxTabDocument.hpp"
#include "WxTabTabulator.hpp"

//#include "utf8.hpp"


BEGIN_EVENT_TABLE(WxTabCanvas, wxScrolledWindow)
	EVT_PAINT  (WxTabCanvas::OnPaint)
	EVT_MOTION (WxTabCanvas::OnMouseMove)
END_EVENT_TABLE()

WxTabCanvas::WxTabCanvas(WxTabFrame *parent)
	: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
	m_owner = parent;
}

void WxTabCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	PrepareDC(dc);

	m_owner->PrepareDC(dc);

	dc.Clear();
	if(theDocument)
		theDocument->Draw(&dc);
}

void WxTabCanvas::OnMouseMove(wxMouseEvent &event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	m_owner->PrepareDC(dc);

	wxPoint pos = event.GetPosition();
	long x = dc.DeviceToLogicalX( pos.x );
	long y = dc.DeviceToLogicalY( pos.y );
	wxString str;
	str.Printf( wxT("Current mouse position: %d,%d"), (int)x, (int)y );
	m_owner->SetStatusText( str );
}




