#ifndef MYFRAME_HPP_INCLUDED
#define MYFRAME_HPP_INCLUDED
#include <wx/wx.h>
#include <wx/spinctrl.h>

class WxTabCanvas;
class PageNumCtrl;
class WxTabFrame : public wxFrame
{
	public:
		WxTabFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
		~WxTabFrame();

		void PrepareDC(wxDC& dc);
		WxTabCanvas   *m_canvas;
	protected:
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnShowTabulatorOptions(wxCommandEvent& event);
		void OnRotate(wxCommandEvent &event);
		void OnMenuGo(wxCommandEvent &event);
    void OnPageSpinCtrl(wxSpinEvent & event);
    void OnOplimitSpinCtrl(wxSpinEvent & event);
		void OnPageNumChanged(wxCommandEvent &event);
	private:
		wxSpinCtrl * m_oplimitspin;
		PageNumCtrl * m_pagenum;
		void AddToolbar();
		DECLARE_EVENT_TABLE()
};

#endif /* MYFRAME_HPP_INCLUDED */

