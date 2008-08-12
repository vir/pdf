#include "pdfdig.hpp"
#include <wx/cmdline.h>
#include "PdfDoc.hpp"
#include "PdfExplorerView.hpp"
 
// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	//// Create a document manager
	m_docManager = new wxDocManager;

	//// Create a template relating drawing documents to their views
	(void) new wxDocTemplate(m_docManager, _T("PDF document"), _T("*.pdf"), _T(""), _T("pdf"), _T("PDF Document"), _T("PDF View"), CLASSINFO(PdfDoc), CLASSINFO(PdfExplorerView));
    
	m_docManager->SetMaxDocsOpen(1);


	if(!wxApp::OnInit()) // it parses command line
		return false;

	frame = new MyFrame(m_docManager, (wxFrame *) NULL, wxID_ANY, _T("PDF Digger"), wxPoint(0, 0), wxSize(500, 400), wxDEFAULT_FRAME_STYLE);

#ifdef __WXMSW__
	//// Give it an icon (this is ignored in MDI mode: uses resources)
	frame->SetIcon(wxIcon(_T("doc_icn")));
#endif

	frame->Centre(wxBOTH);
	frame->Show(true);
	SetTopWindow(frame);
	return true;
}

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
//	{ wxCMD_LINE_PARAM,  NULL, NULL, wxT("input file"), wxCMD_LINE_VAL_STRING, /*wxCMD_LINE_PARAM_MULTIPLE*/wxCMD_LINE_OPTION_MANDATORY },
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
	if(parser.GetParamCount() == 1) {
		fname = parser.GetParam(0);
		if(! theDoc->open(std::string(fname.mb_str())) )
			return false;
	} else {
		return false;
	}
#endif
	return true;
}


