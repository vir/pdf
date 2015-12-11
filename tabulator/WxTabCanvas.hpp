#ifndef MYCANVAS_HPP_INCLUDED
#define MYCANVAS_HPP_INCLUDED
#include <wx/wx.h>

class WxTabFrame;
class CanvasScrollState;
class WxTabCanvas: public wxScrolledWindow
{
	public:
		WxTabCanvas( WxTabFrame *parent);
		void SetScrolledPageSize();
		void OnPaint(wxPaintEvent &event);
		void OnMouse(wxMouseEvent &event);
	private:
		WxTabFrame *m_owner;
		CanvasScrollState *m_scrolling;
		DECLARE_EVENT_TABLE()
};


#endif /* MYCANVAS_HPP_INCLUDED */

