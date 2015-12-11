#ifndef WXTABCANVAS_HPP_INCLUDED
#define WXTABCANVAS_HPP_INCLUDED

#include "../wxview/MyCanvas.hpp"

class WxTabFrame;
class WxTabCanvas: public MyCanvas
{
	public:
		WxTabCanvas( WxTabFrame *parent);
		void OnPaint(wxPaintEvent &event);
		void SetScrolledPageSize();
	private:
		WxTabFrame * m_my_frame;
		DECLARE_EVENT_TABLE()
};

#endif /* WXTABCANVAS_HPP_INCLUDED */

