#ifndef MYFRAME_HPP_INCLUDED
#define MYFRAME_HPP_INCLUDED
#include <wx/wx.h>

class MyCanvas;
class MyFrame : public wxFrame
{
	public:
		MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnRotate(wxCommandEvent &event);

		void PrepareDC(wxDC& dc);
		MyCanvas   *m_canvas;
	private:
		DECLARE_EVENT_TABLE()
};

#endif /* MYFRAME_HPP_INCLUDED */

