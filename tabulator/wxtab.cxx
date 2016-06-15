#include <wx/wx.h>
#include <wx/cmdline.h>
#include "WxTabFrame.hpp"
#include "WxTabDocument.hpp"
#include "WxTabTabulator.hpp"
#include <locale>

class MyApp : public wxApp
{
	private:
		wxString fname;
		unsigned int startpage;
		bool need_gui;
	public:
		virtual bool OnInit();
		virtual int OnExit() { return 0; }
		virtual void OnInitCmdLine(wxCmdLineParser& parser);
		virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
};

IMPLEMENT_APP(MyApp)

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	startpage = 1;
	need_gui = true;

	if(!wxApp::OnInit()) // parses command line
		return false;

	if(!fname.IsEmpty()) {
		theDocument = new WxTabDocument();
		theDocument->Open(fname, startpage);
	} else {
	    theDocument = NULL;
	}

	if(need_gui) {
		WxTabFrame *frame = new WxTabFrame(_T("My PDF tables extractor"), wxPoint(0, 0), wxSize(1000, 750));
		SetTopWindow(frame);
		frame->Show(true);
	}
	return true;
}

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{ wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_OPTION, "p", "page", "start page", wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_PARAM,  NULL, NULL, "input file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL/*wxCMD_LINE_PARAM_MULTIPLE*//*wxCMD_LINE_OPTION_MANDATORY*/ },
	{ wxCMD_LINE_NONE }
};

void MyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(g_cmdLineDesc);
	parser.SetSwitchChars(wxT("-"));
}


bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	parser.Found(wxT("p"), (long int *)&startpage);
	if(parser.GetParamCount() == 1) {
		fname = parser.GetParam(0);
	}
	return true;
}


