#ifndef MYCANVAS_HPP_INCLUDED
#define MYCANVAS_HPP_INCLUDED

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <wx/wx.h>

class MyFrame;
class MyCanvas: public wxScrolledWindow
{
	public:
		MyCanvas( MyFrame *parent);
		void OnPaint(wxPaintEvent &event);
		void OnMouseMove(wxMouseEvent &event);
		void Rotate(int quas) { m_rotation = quas; Refresh(); }
		void debug(bool d) { m_debug = d; }
	private:
		int m_rotation;
		bool m_debug;
		MyFrame *m_owner;
		DECLARE_EVENT_TABLE()
};


#endif /* MYCANVAS_HPP_INCLUDED */

