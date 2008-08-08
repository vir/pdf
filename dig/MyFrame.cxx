#include "MyFrame.hpp"
#include "MyTree.hpp"

enum {
	File_Quit = wxID_EXIT,
	File_About = wxID_ABOUT,
};

const int ID_TOOLBAR = 500;

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU      (File_Quit,     MyFrame::OnQuit)
	EVT_MENU      (File_About,    MyFrame::OnAbout)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
	m_splitter = new wxSplitterWindow(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ );
	m_splitter->SetSashGravity(1.0);

	m_tree = new MyTree(m_splitter);
	m_right = new wxTextCtrl(m_splitter, wxID_ANY, _T("second text"));

	// you can also do this to start with a single window
#if 0
	m_right->Show(false);
	m_splitter->Initialize(m_left);
#else
	// you can also try -100
	m_splitter->SplitVertically(m_tree, m_right, 100);
#endif

	/* Add Menu */
	wxMenu *menuFile = new wxMenu;
	menuFile->AppendSeparator();
	menuFile->Append(File_About, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	menuFile->AppendSeparator();
	menuFile->Append(File_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, _T("&File"));

	SetMenuBar(menuBar);

	/* Add Toolbar */
	AddToolbar();

	/* Add Status Bar */
	CreateStatusBar(2);
	SetStatusText(_T("Welcome to wxWidgets!"));
}

MyFrame::~MyFrame()
{
	delete m_tree;
	delete m_splitter;
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxString msg;
	msg.Printf(
		wxT("This is my humble attempt to draw PDF page.\n")
		wxT("Sorry for bugs.")
	);
	wxMessageBox(msg, _T("About this buggy application"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::AddToolbar()
{
	wxToolBarBase *toolBar = CreateToolBar(wxTB_FLAT/* | wxTB_DOCKABLE*/ | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS, ID_TOOLBAR);

	toolBar->AddTool(wxID_EXIT, _T("Exit"), wxBitmap(), _T("Exit application"));
//    toolBar->AddTool(wxID_OPEN, _T("Open"), toolBarBitmaps[Tool_open], _T("Open file"));

	toolBar->Realize();
}


