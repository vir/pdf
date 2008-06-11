#ifndef MYCANVAS_HPP_INCLUDED
#define MYCANVAS_HPP_INCLUDED
#include <wx/wx.h>

class WxTabFrame;
class WxTabCanvas: public wxScrolledWindow
{
	public:
		WxTabCanvas( WxTabFrame *parent);
		void OnPaint(wxPaintEvent &event);
		void OnMouseMove(wxMouseEvent &event);
	private:
		WxTabFrame *m_owner;
		DECLARE_EVENT_TABLE()
};


#endif /* MYCANVAS_HPP_INCLUDED */

