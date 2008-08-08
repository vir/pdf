#ifndef MYFRAME_HPP_INCLUDED
#define MYFRAME_HPP_INCLUDED
#include <wx/wx.h>
//#include <wx/spinctrl.h>
#include "wx/splitter.h"

class MyTree;
class MyFrame : public wxFrame
{
	public:
		MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
		~MyFrame();
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);

		wxSplitterWindow * m_splitter;
		MyTree * m_tree;
    wxTextCtrl * m_right;
	private:
		void AddToolbar();
		DECLARE_EVENT_TABLE()
};

#endif /* MYFRAME_HPP_INCLUDED */

