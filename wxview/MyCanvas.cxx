#include "MyFrame.hpp"
#include "MyCanvas.hpp"

BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
	EVT_PAINT  (MyCanvas::OnPaint)
	EVT_MOTION (MyCanvas::OnMouseMove)
END_EVENT_TABLE()

MyCanvas::MyCanvas(MyFrame *parent)
	: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
	m_owner = parent;
}

void MyCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	PrepareDC(dc);

	m_owner->PrepareDC(dc);

	dc.Clear();

	dc.DrawText(wxT("Testing"), 40, 60); 
	dc.SetFont( *wxSWISS_FONT );
	dc.DrawRotatedText(wxT("Rotated text") , 40, 80, 45);
//	wxPen pen = *wxRED_PEN;
//	dc.SetPen(pen);
	dc.SetPen( wxPen( wxT("red"), 8 /* width */, wxSOLID) );
	dc.DrawLine( 10, 30,70, 90 );
}

void MyCanvas::OnMouseMove(wxMouseEvent &event)
{
#if wxUSE_STATUSBAR
	wxClientDC dc(this);
	PrepareDC(dc);
	m_owner->PrepareDC(dc);

	wxPoint pos = event.GetPosition();
	long x = dc.DeviceToLogicalX( pos.x );
	long y = dc.DeviceToLogicalY( pos.y );
	wxString str;
	str.Printf( wxT("Current mouse position: %d,%d"), (int)x, (int)y );
	m_owner->SetStatusText( str );
#else
	wxUnusedVar(event);
#endif // wxUSE_STATUSBAR
}
