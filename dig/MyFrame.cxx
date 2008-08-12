#include "MyFrame.hpp"

enum {
	File_Quit = wxID_EXIT,
	File_About = wxID_ABOUT,
};

const int ID_TOOLBAR = 500;


IMPLEMENT_CLASS(MyFrame, wxDocParentFrame)
BEGIN_EVENT_TABLE(MyFrame, wxDocParentFrame)
	EVT_MENU      (File_Quit,     MyFrame::OnQuit)
	EVT_MENU      (File_About,    MyFrame::OnAbout)
END_EVENT_TABLE()

MyFrame::MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title,
                 const wxPoint& pos, const wxSize& size, const long type):
wxDocParentFrame(manager, frame, id, title, pos, size, type)
{
	menuEdit = (wxMenu *) NULL;
	/* Add Menu */
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(wxID_NEW, _T("&New..."));
	menuFile->Append(wxID_OPEN, _T("&Open..."));
	menuFile->Append(wxID_CLOSE, _T("&Close"));
	menuFile->Append(wxID_SAVE, _T("&Save"));
	menuFile->Append(wxID_SAVEAS, _T("Save &As..."));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_PRINT, _T("&Print..."));
	menuFile->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
	menuFile->Append(wxID_PREVIEW, _T("Print Pre&view"));
	menuFile->AppendSeparator();
	menuFile->Append(File_About, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	menuFile->AppendSeparator();
	menuFile->Append(File_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));
	m_docManager->FileHistoryUseMenu(menuFile);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, _T("&File"));

	SetMenuBar(menuBar);


#if 0
    wxMenu *edit_menu = (wxMenu *) NULL;
    
    if (isCanvas)
    {
        edit_menu = new wxMenu;
        edit_menu->Append(wxID_UNDO, _T("&Undo"));
        edit_menu->Append(wxID_REDO, _T("&Redo"));
        edit_menu->AppendSeparator();
        edit_menu->Append(DOCVIEW_CUT, _T("&Cut last segment"));
        
        doc->GetCommandProcessor()->SetEditMenu(edit_menu);
    }
    
    wxMenu *help_menu = new wxMenu;
    help_menu->Append(DOCVIEW_ABOUT, _T("&About"));
    
    wxMenuBar *menu_bar = new wxMenuBar;
    
    menu_bar->Append(file_menu, _T("&File"));
    if (isCanvas)
        menu_bar->Append(edit_menu, _T("&Edit"));
    menu_bar->Append(help_menu, _T("&Help"));

#endif
	/* Add Toolbar */
	AddToolbar();

	/* Add Status Bar */
	CreateStatusBar(2);
	SetStatusText(_T("Welcome to wxWidgets!"));
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


