#ifndef MYFRAME_HPP_INCLUDED
#define MYFRAME_HPP_INCLUDED

#include <wx/wx.h>
#include <wx/aui/aui.h>

class MyCanvas;
class wxSpinCtrl;
class wxSpinEvent;
class MyFrame : public wxFrame
{
	public:
		MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
		~MyFrame();
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnDumpPage(wxCommandEvent& event);
		void OnRotate(wxCommandEvent &event);

		void PrepareDC(wxDC& dc);
		MyCanvas   *m_canvas;
	protected:
    void OnPageSpinCtrl(wxSpinEvent& event);
    void OnOplimitSpinCtrl(wxSpinEvent& event);
	private:
		wxSpinCtrl * m_pagespin;
		wxSpinCtrl * m_oplimitspin;
		wxAuiManager m_mgr;
		void AddToolbar();
		DECLARE_EVENT_TABLE()
};

#endif /* MYFRAME_HPP_INCLUDED */

