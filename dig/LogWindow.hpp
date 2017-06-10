 #ifndef LOGWINDOW_HPP_INCLUDED
 #define LOGWINDOW_HPP_INCLUDED
 #include "wx/wx.h"
 #include <wx/textctrl.h>
 

#if wxHAS_TEXT_WINDOW_STREAM
class LogWindow: public wxFrame
{
public:
	LogWindow():wxFrame(NULL, wxID_ANY, _T("Log"))
	{
		text = new wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP|wxTE_NOHIDESEL);
		redir1 = new wxStreamToTextRedirector(text, &std::cout);
		redir2 = new wxStreamToTextRedirector(text, &std::cerr);
		redir3 = new wxStreamToTextRedirector(text, &std::clog);
	}
	~LogWindow()
	{
		delete redir3;
		delete redir2;
		delete redir1;
		delete text;
	}
	wxTextCtrl* text;
	wxStreamToTextRedirector* redir1;
	wxStreamToTextRedirector* redir2;
	wxStreamToTextRedirector* redir3;
};
#endif

#endif /* LOGWINDOW_HPP_INCLUDED */


