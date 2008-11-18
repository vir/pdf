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

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
#if 0
	std::locale loc("ru_RU.KOI8-R");
	std::wcout.imbue(loc);
	std::wcerr.imbue(loc);
	std::wclog.imbue(loc);
//	std::wclog << L"Привет! Работает, гадина!" << std::endl;
#endif

	startpage = 1;
	need_gui = true;

	if(!wxApp::OnInit()) // parses command line
		return false;

	if(fname) {
		theDocument = new WxTabDocument();
		theDocument->Open(fname, startpage);
	}

	if(need_gui) {
		WxTabFrame *frame = new WxTabFrame(_T("My PDF tables extractor"), wxPoint(0, 0), wxSize(1000, 750));
		frame->Show(true);
		SetTopWindow(frame);
	}
	return true;
}

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_OPTION, wxT("p"), wxT("page"), wxT("start page"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
//	{ wxCMD_LINE_SWITCH, wxT("t"), wxT("test"), wxT("test switch"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_MANDATORY  },
//	{ wxCMD_LINE_SWITCH, wxT("s"), wxT("silent"), wxT("disables the GUI") }, 
	{ wxCMD_LINE_PARAM,  NULL, NULL, wxT("input file"), wxCMD_LINE_VAL_STRING, /*wxCMD_LINE_PARAM_MULTIPLE*/wxCMD_LINE_OPTION_MANDATORY },
	{ wxCMD_LINE_NONE }
};

void MyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(g_cmdLineDesc);
	parser.SetSwitchChars(wxT("-"));
}


bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
#if 0
	silent_mode = parser.Found(wxT("s"));
	wxArrayString files;
	for (int i = 0; i < parser.GetParamCount(); i++) { files.Add(parser.GetParam(i)); }
#endif
	parser.Found(wxT("p"), (long int *)&startpage);
	if(parser.GetParamCount() == 1) {
		fname = parser.GetParam(0);
	} else {
		return false;
	}
	return true;
}


