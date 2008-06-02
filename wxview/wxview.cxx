#include <wx/wx.h>
#include <wx/cmdline.h>
#include "MyFrame.hpp"
#include "MyDocument.hpp"
 
class MyApp : public wxApp
{
	private:
		std::string fname;
		unsigned int startpage;
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
	startpage = 1;
	if(!wxApp::OnInit()) // it parses command line
		return false;
	theDocument = new MyDocument();
	if(! theDocument->LoadFile(fname)) return false;
	if(! theDocument->LoadPage(startpage)) return false;
	MyFrame *frame = new MyFrame(_T("My Buggy PDF Viewer"), wxPoint(10, 10), wxSize(1100, 1000));
	frame->Show(true);
	SetTopWindow(frame);
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
std::cerr << "Params count: " << parser.GetParamCount() << std::endl;
	if(parser.GetParamCount() == 1) {
		fname = parser.GetParam(0).mb_str();
	} else {
		return false;
	}
	return true;
}


