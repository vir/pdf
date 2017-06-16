#ifndef LOGWINDOW_HPP_INCLUDED
#define LOGWINDOW_HPP_INCLUDED

#include <wx/wx.h>

#if wxHAS_TEXT_WINDOW_STREAM
class LogWindow : public wxFrame
{
public:
	LogWindow() :wxFrame(NULL, wxID_ANY, _T("Log"))
	{
		fixedFont = new wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		text = new wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_NOHIDESEL);
		text->SetFont(*fixedFont);
		redir1 = new wxStreamToTextRedirector(text, &std::cout);
		redir2 = new wxStreamToTextRedirector(text, &std::cerr);
		redir3 = new wxStreamToTextRedirector(text, &std::clog);
	}
	~LogWindow()
	{
		delete redir1, redir2, redir3;
		delete text;
		delete fixedFont;
	}
	wxTextCtrl* text;
	wxFont* fixedFont;
	wxStreamToTextRedirector* redir1;
	wxStreamToTextRedirector* redir2;
	wxStreamToTextRedirector* redir3;
};
#endif

#endif /* LOGWINDOW_HPP_INCLUDED */

