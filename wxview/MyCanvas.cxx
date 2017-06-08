#include "MyCanvas.hpp"
#include <iomanip> /* for setiosflags and so */
#include "../PDF.hpp"
#include "PagePaintHelper.h"

BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
	EVT_PAINT  (MyCanvas::OnPaint)
	EVT_MOTION (MyCanvas::OnMouse)
	EVT_LEFT_DOWN(MyCanvas::OnMouse)
	EVT_LEFT_UP(MyCanvas::OnMouse)
END_EVENT_TABLE()

MyCanvas::MyCanvas(wxWindow *parent)
	: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
	, m_break(false)
{
	m_owner = parent;
	m_frame = NULL;
	m_scrolling = NULL;
	m_rotation = 0;
	m_scale = 1.0;
}

void MyCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	PrepareDC(dc);

	m_owner->PrepareDC(dc);

	dc.Clear();

	PagePaintHelper m(dc, m_rotation, m_scale);
	if(m_debug) {
		m_pagelog.str("");
		m.set_draw_debug_stream(&m_pagelog);
		m.set_page_debug_stream(&m_pagelog);
	}
	unsigned int lim = m_page->get_operators_number_limit();
	if(lim && m_break)
		m.set_break(lim - 1);
	m_break = false;
	m_page->draw(&m);
}

void MyCanvas::SetPageSize()
{
	PDF::Rect mb = m_page->get_meadia_box();
	int w = mb.x2 + mb.x1;
	int h = mb.y2 + mb.y1; // Add some margins
	w *= m_scale;
	h *= m_scale;
	switch(m_rotation)
	{
	case 1: case 3: std::swap(w, h);
	default: break;
	}

	SetVirtualSize(w, h);
	SetScrollbars( 1, 1, w, h);
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

void MyCanvas::OnMouse(wxMouseEvent &event)
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

	if(m_frame)
	{
		x /= m_scale;
		y /= m_scale;
		wxString str;
		str.Printf( wxT("Current mouse position: %d,%d "), (int)x, (int)y );
		if(m_scrolling)
			str += m_scrolling->dump();
		m_frame->SetStatusText( str );
	}
}

