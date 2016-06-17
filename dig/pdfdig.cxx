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
	::wxInitAllImageHandlers();

	SetVendorName(_T("vir"));
	SetAppName(_T("pdfdig")); // not needed, it's the default value
	SetAppDisplayName("PDF Digger");

	//wxConfigBase *pConfig = wxConfigBase::Get();

	// uncomment this to force writing back of the defaults for all values
	// // if they're not present in the config - this can give the user an idea
	// // of all possible settings for this program
	//pConfig->SetRecordDefaults();

	//delete wxLog::SetActiveTarget(new wxLogStderr);

	//// Create a document manager
	m_docManager = new wxDocManager;
	m_docManager->FileHistoryLoad(*wxConfig::Get());

	//// Create a template relating drawing documents to their views
	(void) new wxDocTemplate(m_docManager, _T("PDF document"), _T("*.pdf"), _T(""), _T("pdf"),
		_T("PDF Document"), _T("PDF View"), CLASSINFO(PdfDoc), CLASSINFO(PdfExplorerView));
    
	//m_docManager->SetMaxDocsOpen(1);

	frame = new MyFrame(m_docManager, (wxFrame *) NULL, wxID_ANY, GetAppDisplayName(), wxDefaultPosition, wxSize(800, 600), wxDEFAULT_FRAME_STYLE);

#ifdef __WXMSW__
	//// Give it an icon (this is ignored in MDI mode: uses resources)
	//frame->SetIcon(wxIcon(_T("doc_icn")));
#endif

	frame->Centre(wxBOTH);
	frame->Show(true);
	//SetTopWindow(frame);

	if (m_filesFromCmdLine.empty())
	{
		//m_docManager->CreateNewDocument();
	}
	else // we have files to open on command line
	{
		for (size_t i = 0; i != m_filesFromCmdLine.size(); ++i)
			m_docManager->CreateDocument(m_filesFromCmdLine[i], wxDOC_SILENT);
	}
	return true;
}

int MyApp::OnExit()
{
	m_docManager->FileHistorySave(*wxConfig::Get());
	delete m_docManager;
	return wxApp::OnExit();
}

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
//	{ wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_PARAM,  NULL, NULL, "input file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_NONE }
};

void MyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	wxApp::OnInitCmdLine(parser);
	parser.SetDesc(g_cmdLineDesc);
#ifdef _MSC_VER
	parser.SetSwitchChars(wxT("/-"));
#else
	parser.SetSwitchChars(wxT("-"));
#endif
}


bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	// save any files given on the command line: we'll open them in OnInit()
	// later, after creating the frame
	for (size_t i = 0; i != parser.GetParamCount(); ++i)
		m_filesFromCmdLine.push_back(parser.GetParam(i));
	return wxApp::OnCmdLineParsed(parser);
}


