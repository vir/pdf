#include "MyFrame.hpp"
#include "MyCanvas.hpp"
#include <iomanip> /* for setiosflags and so */
#include "../PDF.hpp"
#include "PagePaintHelper.h"

BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
	EVT_PAINT  (MyCanvas::OnPaint)
	EVT_MOTION (MyCanvas::OnMouseMove)
END_EVENT_TABLE()

MyCanvas::MyCanvas(wxWindow *parent)
	: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
	m_owner = parent;
	m_rotation = 0;
}

void MyCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	PrepareDC(dc);

	m_owner->PrepareDC(dc);

	dc.Clear();

	PagePaintHelper m(dc, m_rotation, 1.0, m_debug);
	m_page->draw(&m);
}

void MyCanvas::OnMouseMove(wxMouseEvent &event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	m_owner->PrepareDC(dc);
#if 0 // XXX different owner in pdfdig!!!
	wxPoint pos = event.GetPosition();
	long x = dc.DeviceToLogicalX( pos.x );
	long y = dc.DeviceToLogicalY( pos.y );
	wxString str;
	str.Printf( wxT("Current mouse position: %d,%d"), (int)x, (int)y );
	static_cast<MyFrame*>(m_owner)->SetStatusText( str );
#endif
}

