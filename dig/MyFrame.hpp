#ifndef MYFRAME_HPP_INCLUDED
#define MYFRAME_HPP_INCLUDED
#include <wx/wx.h>
#include "wx/docview.h"

class MyFrame:public wxDocParentFrame
{
	DECLARE_CLASS(MyFrame)
	public:
		MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type);
		~MyFrame();
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);

		wxMenu * menuEdit;
	private:
		void AddToolbar();
		DECLARE_EVENT_TABLE()
};

#endif /* MYFRAME_HPP_INCLUDED */

