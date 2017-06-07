#ifndef WXTABFRAME_HPP_INCLUDED
#define WXTABFRAME_HPP_INCLUDED
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "wx/aui/aui.h"
#include "ErrorReporterInterface.hpp"

class WxTabCanvas;
class PageNumCtrl;
class WxTabDocument;
struct BatchExport
{
	BatchExport(wxWindow* parent): m_parent(parent), m_firstpage(NULL), m_lastpage(NULL), m_pages_in_row(NULL) { }
	wxAuiToolBar* CreateToolBar();
	void OnCmd(wxCommandEvent &event);
	wxWindow * m_parent;
	PageNumCtrl * m_firstpage;
	PageNumCtrl * m_lastpage;
	wxSpinCtrl* m_pages_in_row;
	wxAuiToolBar* CreateToolBar(wxWindow * parent, WxTabDocument * doc);
};
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
		void OnBatchCmd(wxCommandEvent & event);
		void OnLogWindow(wxCommandEvent& event);
public: // ErrorReporter
		virtual void ReportError(const char * prefix, const char * msg);
	private:
		wxAuiManager m_mgr;
//		wxSpinCtrl * m_oplimitspin;
		PageNumCtrl * m_pagenum;
		BatchExport * m_batch_export;
		wxFrame * logWindow;
		DECLARE_EVENT_TABLE()
};

#endif /* WXTABFRAME_HPP_INCLUDED */

