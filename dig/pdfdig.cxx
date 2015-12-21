#include "pdfdig.hpp"
#include <wx/cmdline.h>
#include "PdfDoc.hpp"
#include "PdfExplorerView.hpp"
#include <wx/config.h>
 
// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	if(!wxApp::OnInit()) // it parses command line
		return false;

	SetVendorName(_T("vir"));
	SetAppName(_T("pdfdig")); // not needed, it's the default value

	//wxConfigBase *pConfig = wxConfigBase::Get();

	// uncomment this to force writing back of the defaults for all values
	// // if they're not present in the config - this can give the user an idea
	// // of all possible settings for this program
	//pConfig->SetRecordDefaults();

	//delete wxLog::SetActiveTarget(new wxLogStderr);

	//// Create a document manager
	m_docManager = new wxDocManager;
	m_docManager->FileHistoryLoad(*wxConfigBase::Get());

	//// Create a template relating drawing documents to their views
	(void) new wxDocTemplate(m_docManager, _T("PDF document"), _T("*.pdf"), _T(""), _T("pdf"),
		_T("PDF Document"), _T("PDF View"), CLASSINFO(PdfDoc), CLASSINFO(PdfExplorerView));
    
	//m_docManager->SetMaxDocsOpen(1);

	frame = new MyFrame(m_docManager, (wxFrame *) NULL, wxID_ANY, _T("PDF Digger"), wxPoint(0, 0), wxSize(800, 600), wxDEFAULT_FRAME_STYLE);

#ifdef __WXMSW__
	//// Give it an icon (this is ignored in MDI mode: uses resources)
	//frame->SetIcon(wxIcon(_T("doc_icn")));
#endif

	frame->Centre(wxBOTH);
	frame->Show(true);
	SetTopWindow(frame);

	if(! fname.empty())
	{
		wxCommandEvent ev();
//		m_docManager->OnFileOpen(ev);
	}
	return true;
}

int MyApp::OnExit()
{
	//XXX m_docManager->FileHistorySave(*wxConfigBase::Get());
	delete m_docManager;
	return wxApp::OnExit();
}

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_PARAM,  NULL, NULL, wxT("input file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_NONE }
};

void MyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(g_cmdLineDesc);
#ifdef _MSC_VER
	parser.SetSwitchChars(wxT("/-"));
#else
	parser.SetSwitchChars(wxT("-"));
#endif
}


bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	if(parser.GetParamCount() == 1) {
		fname = parser.GetParam(0);
//		if(! theDoc->open(std::string(fname.mb_str())) )
//			return false;
	}
	return true;
}


