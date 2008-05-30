#ifndef MYCANVAS_HPP_INCLUDED
#define MYCANVAS_HPP_INCLUDED
#include <wx/wx.h>

class MyFrame;
class MyCanvas: public wxScrolledWindow
{
	public:
		MyCanvas( MyFrame *parent);
		void OnPaint(wxPaintEvent &event);
		void OnMouseMove(wxMouseEvent &event);
		void Rotate(int quas) { m_rotation = quas; Refresh(); }
	private:
		int m_rotation;
		MyFrame *m_owner;
		DECLARE_EVENT_TABLE()
};


#endif /* MYCANVAS_HPP_INCLUDED */
