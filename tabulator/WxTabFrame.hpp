#ifndef WXTABFRAME_HPP_INCLUDED
#define WXTABFRAME_HPP_INCLUDED
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "wx/aui/aui.h"
#include "ErrorReporterInterface.hpp"

class WxTabCanvas;
class PageNumCtrl;
class WxTabFrame : public wxFrame, public ErrorReporter
{
	public:
		WxTabFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
		~WxTabFrame();

		void PrepareDC(wxDC& dc);
		WxTabCanvas   *m_canvas;
	protected:
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnDocumentOpen(wxCommandEvent& event);
		void OnShowTabulatorOptions(wxCommandEvent& event);
		void OnRotate(wxCommandEvent &event);
		void OnZoom(wxCommandEvent &event);
		void OnMenuGo(wxCommandEvent &event);
		void OnMenuExport(wxCommandEvent &event);
		void OnBatchExport(wxCommandEvent &event);
		void OnOplimitSpinCtrl(wxSpinEvent & event);
		void OnPageNumChanged(wxCommandEvent &event);
	public: // ErrorReporter
		virtual void ReportError(const char * prefix, const char * msg);
	private:
		wxAuiManager m_mgr;
//		wxSpinCtrl * m_oplimitspin;
		PageNumCtrl * m_pagenum;
		DECLARE_EVENT_TABLE()
};

#endif /* WXTABFRAME_HPP_INCLUDED */

