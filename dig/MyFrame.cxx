#include "MyFrame.hpp"
#include "pdfglass.xpm"

enum {
	File_Quit = wxID_EXIT,
	File_About = wxID_ABOUT,
};

const int ID_TOOLBAR = 500;


IMPLEMENT_CLASS(MyFrame, wxDocMDIParentFrame)
BEGIN_EVENT_TABLE(MyFrame, wxDocMDIParentFrame)
	EVT_MENU      (File_Quit,     MyFrame::OnQuit)
	EVT_MENU      (File_About,    MyFrame::OnAbout)
END_EVENT_TABLE()

MyFrame::MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title,
                 const wxPoint& pos, const wxSize& size, const long type)
	: wxDocMDIParentFrame(manager, frame, id, title, pos, size, type)
{
	SetMinSize(wxSize(400,300));
#if 0
	m_mgr.SetFlags(m_mgr.GetFlags()
		| wxAUI_MGR_ALLOW_FLOATING
		| wxAUI_MGR_TRANSPARENT_DRAG
		| wxAUI_MGR_HINT_FADE
		| wxAUI_MGR_TRANSPARENT_HINT
	);
	// tell wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);
#endif
	SetIcon(wxIcon(pdfglass_xpm));

	/* Add Menu */
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(wxID_OPEN, _T("&Open..."));
	menuFile->Append(wxID_CLOSE, _T("&Close"));
	menuFile->AppendSeparator();
	menuFile->Append(File_About, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	menuFile->AppendSeparator();
	menuFile->Append(File_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));
	//m_docManager->FileHistoryUseMenu(menuFile);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, _T("&File"));
	SetMenuBar(menuBar);

	wxToolBarBase *toolBar = CreateToolBar(wxTB_FLAT/* | wxTB_DOCKABLE*/ | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS, ID_TOOLBAR);
	toolBar->AddTool(wxID_EXIT, _T("Exit"), wxBitmap(), _T("Exit application"));
	toolBar->AddTool(wxID_OPEN, _T("Open"), wxBitmap(), _T("Open file"));
	toolBar->Realize();

	CreateStatusBar(2);
	SetStatusText(_T("pdfdig ready to rock"));

	SetMinSize(wxSize(400,300));
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxString msg;
	msg.Printf(
		wxT("Pdf file exploration tool.\n")
		wxT("Sorry for bugs.")
	);
	wxMessageBox(msg, _T("About this buggy application"), wxOK | wxICON_INFORMATION, this);
}



