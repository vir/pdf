#include "WxTabFrame.hpp"
#include "WxTabCanvas.hpp"
//#include <iomanip> /* for setiosflags and so */
//#include "../PDF.hpp"
#include "WxTabDocument.hpp"
#include "WxTabTabulator.hpp"

//#include "utf8.hpp"

BEGIN_EVENT_TABLE(WxTabCanvas, wxScrolledWindow)
	EVT_PAINT  (WxTabCanvas::OnPaint)
	EVT_MOTION (WxTabCanvas::OnMouse)
	EVT_LEFT_DOWN(WxTabCanvas::OnMouse)
	EVT_LEFT_UP(WxTabCanvas::OnMouse)
END_EVENT_TABLE()

WxTabCanvas::WxTabCanvas(WxTabFrame *parent)
	: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
	m_owner = parent;
	m_scrolling = NULL;
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

void WxTabCanvas::SetScrolledPageSize()
{
	if(! theDocument)
		return;
	wxSize s(theDocument->GetPageSize());
	SetVirtualSize(s);
	SetScrollbars( 1, 1, s.GetWidth(), s.GetHeight());
}

class CanvasScrollState
{
public:
	CanvasScrollState(wxScrolledWindow* w): w(w) { }
	void Start(long x, long y)
	{
		w->GetViewStart(&view_start_x, &view_start_y);
		mouse_start_x = x;
		mouse_start_y = y;
		vx = view_start_x;
		vy = view_start_y;
	}
	void Move(long x, long y)
	{
	}
	void Finish(long x, long y)
	{
		long diff_x = x - mouse_start_x;
		long diff_y = y - mouse_start_y;
		vx = view_start_x - diff_x;
		vy = view_start_y - diff_y;
		if(vx < 0)
			vx = 0;
		if(vy < 0)
			vy = 0;
		w->Scroll(vx, vy);
	}
	wxString dump()
	{
		wxString r;
		r.Printf(wxT("start(%d, %d), view(%d, %d), scroll(%d,%d)"), mouse_start_x, mouse_start_y, view_start_x, view_start_y, vx, vy);
		return r;
	}
private:
	wxScrolledWindow* w;
	long mouse_start_x, mouse_start_y;
	int view_start_x, view_start_y;
	int vx, vy;
};

void WxTabCanvas::OnMouse(wxMouseEvent &event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	m_owner->PrepareDC(dc);

	wxPoint pos = event.GetPosition();
	long x = dc.DeviceToLogicalX( pos.x );
	long y = dc.DeviceToLogicalY( pos.y );

	if(event.GetEventType() == wxEVT_LEFT_DOWN)
	{
		if(! m_scrolling)
			m_scrolling = new CanvasScrollState(this);
		m_scrolling->Start(x, y);
	}
	else if(event.GetEventType() == wxEVT_LEFT_UP)
	{
		if(m_scrolling)
			m_scrolling->Finish(x, y);
		delete m_scrolling;
		m_scrolling = NULL;
	}
	else if(event.GetEventType() == wxEVT_MOTION)
	{
		if(m_scrolling)
			m_scrolling->Move(x, y);
	}

	wxString str;
	str.Printf( wxT("Current mouse position: %d,%d "), (int)x, (int)y );
	if(m_scrolling)
		str += m_scrolling->dump();
	m_owner->SetStatusText( str );

}




