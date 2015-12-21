#ifndef MYCANVAS_HPP_INCLUDED
#define MYCANVAS_HPP_INCLUDED

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <wx/wx.h>
#include <sstream>

namespace PDF { class Page; }

class CanvasScrollState;

class MyCanvas: public wxScrolledWindow
{
	public:
		MyCanvas( wxWindow *parent);
		void SetFrame(wxFrameBase* frame) { m_frame = frame; }
		void SetPage(PDF::Page* page) { m_page = page; SetPageSize(); }
		void SetPageSize();
		void OnPaint(wxPaintEvent &event);
		void OnMouse(wxMouseEvent &event);
		void Rotate(int quas) { m_rotation = quas; Refresh(); }
		void debug(bool d) { m_debug = d; }
		std::string get_page_log() const { return m_pagelog.str(); }
		void set_break() { m_break = true; }
		void set_scale(double scale) { m_scale = scale; }
	private:
		PDF::Page* m_page;
		int m_rotation;
		double m_scale;
		bool m_debug;
		bool m_break;
		std::ostringstream m_pagelog;
		CanvasScrollState *m_scrolling;
		wxWindowBase *m_owner;
		wxFrameBase *m_frame;
		DECLARE_EVENT_TABLE()
};


#endif /* MYCANVAS_HPP_INCLUDED */

