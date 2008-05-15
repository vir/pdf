#include "MyFrame.hpp"
#include "MyCanvas.hpp"

enum {
	File_Quit = wxID_EXIT,
	File_About = wxID_ABOUT,

	MenuRotate_First = wxID_HIGHEST,
	Rotate_0 = MenuRotate_First,
	Rotate_90,
	Rotate_180,
	Rotate_270,
	MenuRotate_Last = Rotate_270,
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU      (File_Quit,     MyFrame::OnQuit)
	EVT_MENU      (File_About,    MyFrame::OnAbout)
	EVT_MENU_RANGE(MenuRotate_First,   MenuRotate_Last,   MyFrame::OnRotate)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
	wxMenu *menuFile = new wxMenu;
	menuFile->AppendSeparator();
	menuFile->Append(File_About, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	menuFile->AppendSeparator();
	menuFile->Append(File_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

	wxMenu * menuRotate = new wxMenu;
	menuRotate->Append(Rotate_0, _T("No rotation\tF1"));
	menuRotate->Append(Rotate_90, _T("Rotate &90 degree\tF2"));
	menuRotate->Append(Rotate_180, _T("Rotate &180 degree\tF3"));
	menuRotate->Append(Rotate_270, _T("Rotate &270 degrees\tF4"));

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, _T("&File"));
	menuBar->Append(menuRotate, _T("&Rotate"));

	SetMenuBar(menuBar);

	CreateStatusBar(2);
	SetStatusText(_T("Welcome to wxWidgets!"));

	m_canvas = new MyCanvas( this );
	m_canvas->SetScrollbars( 10, 10, 100, 240 );
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

void MyFrame::OnRotate(wxCommandEvent& event)
{
//    m_canvas->DoRotate((event.GetId() - MenuRotate_First));
}

void MyFrame::PrepareDC(wxDC& dc)
{
#if 0
	dc.SetLogicalOrigin( m_xLogicalOrigin, m_yLogicalOrigin );
	dc.SetAxisOrientation( !m_xAxisReversed, m_yAxisReversed );
	dc.SetUserScale( m_xUserScale, m_yUserScale );
	dc.SetMapMode( m_mapMode );
#endif
}

