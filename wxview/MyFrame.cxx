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

const int ID_TOOLBAR = 500;
const int ID_SPIN_PAGE = 510;
const int ID_SPIN_OPLIMIT = 511;

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU      (File_Quit,     MyFrame::OnQuit)
	EVT_MENU      (File_About,    MyFrame::OnAbout)
	EVT_MENU_RANGE(MenuRotate_First,   MenuRotate_Last,   MyFrame::OnRotate)
	EVT_SPINCTRL  (ID_SPIN_PAGE,  MyFrame::OnPageSpinCtrl)
	EVT_SPINCTRL  (ID_SPIN_OPLIMIT, MyFrame::OnOplimitSpinCtrl)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
	m_pagespin = NULL;
	m_oplimitspin = NULL;
	m_canvas = NULL;

	/* Add Menu */
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

	/* Add Toolbar */
	AddToolbar();

	/* Add Status Bar */
	CreateStatusBar(2);
	SetStatusText(_T("Welcome to wxWidgets!"));

	/* And finally owr main widget */
	m_canvas = new MyCanvas( this );
	m_canvas->SetScrollbars( 10, 10, 100, 240 );
}

MyFrame::~MyFrame()
{
	delete m_oplimitspin;
	delete m_pagespin;
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
	m_canvas->Rotate(event.GetId() - MenuRotate_First);
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

#include "MyDocument.hpp"

void MyFrame::AddToolbar()
{
	wxToolBarBase *toolBar = CreateToolBar(wxTB_FLAT/* | wxTB_DOCKABLE*/ | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS, ID_TOOLBAR);

	m_pagespin = new wxSpinCtrl( toolBar, ID_SPIN_PAGE, _T("")/*, wxDefaultPosition, wxSize(40,wxDefaultCoord)*/ );
	m_pagespin->SetRange(1, theDocument->GetPagesNum());
	m_pagespin->SetValue(theDocument->GetPageNum());
	toolBar->AddControl(m_pagespin);

	toolBar->AddTool(wxID_EXIT, _T("Exit"), wxBitmap(), _T("Exit application"));
//    toolBar->AddTool(wxID_OPEN, _T("Open"), toolBarBitmaps[Tool_open], _T("Open file"));

	m_oplimitspin = new wxSpinCtrl( toolBar, ID_SPIN_OPLIMIT, _T("")/*, wxDefaultPosition, wxSize(40,wxDefaultCoord)*/ );
	m_oplimitspin->SetRange(0, 10000);
	m_oplimitspin->SetValue(0);
	toolBar->AddControl(m_oplimitspin);

	toolBar->Realize();
}

void MyFrame::OnPageSpinCtrl(wxSpinEvent& event)
{
	if(m_pagespin && m_canvas) {
		theDocument->LoadPage(m_pagespin->GetValue());
		std::clog << "Page switched to " << m_pagespin->GetValue() << std::endl;
		m_canvas->Refresh();
	}
}

void MyFrame::OnOplimitSpinCtrl(wxSpinEvent& event)
{
	if(m_oplimitspin && m_canvas) {
		PDF::Page * p = theDocument->GetPageObject();
		int lim = m_oplimitspin->GetValue();
		p->set_operators_number_limit(lim);
		m_canvas->debug(lim > 0);
		m_canvas->Refresh();
	}
}

