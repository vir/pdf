#ifndef MYCANVAS_HPP_INCLUDED
#define MYCANVAS_HPP_INCLUDED

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <wx/wx.h>

class MyFrame;
namespace PDF { class Page; }

class MyCanvas: public wxScrolledWindow
{
	public:
		MyCanvas( wxWindow *parent);
		void SetPage(PDF::Page* page) { m_page = page; }
		void OnPaint(wxPaintEvent &event);
		void OnMouseMove(wxMouseEvent &event);
		void Rotate(int quas) { m_rotation = quas; Refresh(); }
		void debug(bool d) { m_debug = d; }
	private:
		PDF::Page* m_page;
		int m_rotation;
		bool m_debug;
		wxWindowBase *m_owner;
		DECLARE_EVENT_TABLE()
};


#endif /* MYCANVAS_HPP_INCLUDED */

