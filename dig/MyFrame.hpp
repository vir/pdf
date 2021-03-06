#ifndef MYFRAME_HPP_INCLUDED
#define MYFRAME_HPP_INCLUDED
#include <wx/wx.h>
#include "wx/docview.h"
//#include <wx/aui/aui.h>
#include <wx/docmdi.h>

class MyFrame:public wxDocMDIParentFrame
{
	DECLARE_CLASS(MyFrame)
	public:
		MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type);
		~MyFrame();
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnViewStream(wxCommandEvent& event);
		void OnSaveStream(wxCommandEvent& event);
		void OnViewPage(wxCommandEvent& event);
		void OnLogWindow(wxCommandEvent& event);
		void OnSaveLog(wxCommandEvent& event);
		void ViewStreamEnable(bool enable);
		void ViewPageEnable(bool enable);
private:
		wxMenu *menuView;
		wxToolBarBase *toolBar;
		wxFrame * logWindow;
		//wxAuiManager m_mgr;
		DECLARE_EVENT_TABLE()
};

#endif /* MYFRAME_HPP_INCLUDED */

