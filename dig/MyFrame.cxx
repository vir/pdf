#include "MyFrame.hpp"
#include "pdfdig.hpp"
#include "PdfExplorerView.hpp" // need to pass event there
#include "pdfglass.xpm"
#include "LogWindow.hpp"
#include <wx/textctrl.h>

enum {
	File_Quit = wxID_EXIT,
	File_About = wxID_ABOUT,
	View_ViewStream = wxID_HIGHEST + 1,
	View_SaveStream,
	View_ViewPage,
	View_LogWindow,
	View_SaveLog,
};

const int ID_TOOLBAR = 500;


IMPLEMENT_CLASS(MyFrame, wxDocMDIParentFrame)
BEGIN_EVENT_TABLE(MyFrame, wxDocMDIParentFrame)
	EVT_MENU      (File_Quit,     MyFrame::OnQuit)
	EVT_MENU      (File_About,    MyFrame::OnAbout)
	EVT_MENU      (View_ViewStream, MyFrame::OnViewStream)
	EVT_MENU      (View_SaveStream, MyFrame::OnSaveStream)
	EVT_MENU      (View_ViewPage, MyFrame::OnViewPage)
	EVT_MENU      (View_LogWindow, MyFrame::OnLogWindow)
	EVT_MENU      (View_SaveLog, MyFrame::OnSaveLog)
END_EVENT_TABLE()

MyFrame::MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title,
                 const wxPoint& pos, const wxSize& size, const long type)
	: wxDocMDIParentFrame(manager, frame, id, title, pos, size, type)
	, logWindow(NULL)
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
	menuFile->Append(File_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));
	m_docManager->FileHistoryUseMenu(menuFile);
	m_docManager->FileHistoryAddFilesToMenu();

	menuView = new wxMenu;
	menuView->Append(View_SaveStream, _T("Save Stream\tF2"), _T("Save stream object content into file"));
	menuView->Append(View_ViewStream, _T("View Stream\tF3"), _T("View stream object content"));
	menuView->Append(View_ViewPage, _T("View Page\tF4"), _T("View page content"));
	menuView->AppendSeparator();
#if wxHAS_TEXT_WINDOW_STREAM
	menuView->AppendCheckItem(View_LogWindow, _T("View log window\tF9"), _T("Show or hide log window"));
#else
	menuView->AppendCheckItem(View_SaveLog, _T("Save log to file...\tF11"), _T("Save log to specified file"));
#endif

	wxMenu *help = new wxMenu;
	help->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, wxGetStockLabel(wxID_FILE));
	menuBar->Append(menuView, _T("&View"));
	menuBar->Append(help, wxGetStockLabel(wxID_HELP));
	SetMenuBar(menuBar);

	toolBar = CreateToolBar(wxTB_FLAT/* | wxTB_DOCKABLE*/ | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS, ID_TOOLBAR);
	toolBar->AddTool(wxID_EXIT, _T("Exit"), wxBitmap(), _T("Exit application"));
	toolBar->AddTool(wxID_OPEN, _T("Open"), wxBitmap(), _T("Open file"));
	toolBar->AddSeparator();
#if wxHAS_TEXT_WINDOW_STREAM
	toolBar->AddCheckTool(View_LogWindow, _T("Log window"), wxBitmap(), wxNullBitmap, _T("Save log window"));
	toolBar->AddSeparator();
#endif
	toolBar->AddTool(View_SaveStream, _T("SaveStream"), wxBitmap(), _T("Save stream object content"));
	toolBar->AddTool(View_ViewStream, _T("ViewStream"), wxBitmap(), _T("View stream object content"));
	toolBar->AddTool(View_ViewPage, _T("ViewPage"), wxBitmap(), _T("View page content"));
	toolBar->Realize();

	CreateStatusBar(2);
	SetStatusText(_T("pdfdig ready to rock"));

	SetMinSize(wxSize(400,300));
	ViewStreamEnable(false);
	ViewPageEnable(false);
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

void MyFrame::OnViewStream(wxCommandEvent& event)
{
	try
	{
		wxView* curview = static_cast<MyApp*>(wxTheApp)->GetDocManager()->GetCurrentView();
		if (curview)
			static_cast<PdfExplorerView*>(curview)->ViewStreamData();
	}
	catch (PDF::FormatException & e) {
		wxMessageBox(wxString(e.what(), wxConvUTF8), _T("PDF::FormatException"), wxOK | wxICON_INFORMATION, this);
	}
	catch (std::exception & e) {
		wxMessageBox(wxString(e.what(), wxConvUTF8), _T("std::exception"), wxOK | wxICON_INFORMATION, this);
	}
	catch (...) {
		wxMessageBox(_T("Unknown Exception"), _T("Exception!"), wxOK | wxICON_INFORMATION, this);
	}
}

void MyFrame::OnViewPage(wxCommandEvent& event)
{
	try
	{
		wxView* curview = static_cast<MyApp*>(wxTheApp)->GetDocManager()->GetCurrentView();
		if(curview)
			static_cast<PdfExplorerView*>(curview)->ViewPage();
	}
	catch(PDF::FormatException & e) {
		wxMessageBox(wxString(e.what(), wxConvUTF8), _T("PDF::FormatException"), wxOK | wxICON_INFORMATION, this);
	}
	catch(std::exception & e) {
		wxMessageBox(wxString(e.what(), wxConvUTF8), _T("std::exception"), wxOK | wxICON_INFORMATION, this);
	}
	catch(...) {
		wxMessageBox(_T("Unknown Exception"), _T("Exception!"), wxOK | wxICON_INFORMATION, this);
	}
}

void MyFrame::OnSaveStream(wxCommandEvent& event)
{
	wxView* curview = static_cast<MyApp*>(wxTheApp)->GetDocManager()->GetCurrentView();
	if(curview)
	{
		wxFileDialog* OpenDialog = new wxFileDialog(
			this, _("Choose name for your file"), wxEmptyString, wxEmptyString, 
			_("Text files (*.txt)|*.txt|C++ Source Files (*.cpp, *.cxx)|*.cpp;*.cxx|All files|*.*"),
			wxFD_SAVE, wxDefaultPosition);
		if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "Cancel"
		{
			try
			{
				wxString SavePath = OpenDialog->GetPath();
				std::ofstream s;
				s.open(SavePath.mb_str(), std::ios_base::out | std::ios_base::binary);
				static_cast<PdfExplorerView*>(curview)->SaveStreamData(s);
			}
			catch (PDF::FormatException & e) {
				wxMessageBox(wxString(e.what(), wxConvUTF8), _T("PDF::FormatException"), wxOK | wxICON_INFORMATION, this);
			}
			catch (std::exception & e) {
				wxMessageBox(wxString(e.what(), wxConvUTF8), _T("std::exception"), wxOK | wxICON_INFORMATION, this);
			}
			catch (...) {
				wxMessageBox(_T("Unknown Exception"), _T("Exception!"), wxOK | wxICON_INFORMATION, this);
			}
		}
		OpenDialog->Destroy();
	}
}

void MyFrame::OnLogWindow(wxCommandEvent& event)
{
#if wxHAS_TEXT_WINDOW_STREAM
	if(event.IsChecked()) {
		logWindow = new LogWindow;
		logWindow->Show(true);
	} else {
		delete logWindow;
		logWindow = NULL;
	}
#endif
}

void MyFrame::ViewStreamEnable(bool enable)
{
	menuView->Enable(View_ViewStream, enable);
	toolBar->EnableTool(View_ViewStream, enable);
	menuView->Enable(View_SaveStream, enable);
	toolBar->EnableTool(View_SaveStream, enable);
}

void MyFrame::ViewPageEnable(bool enable)
{
	menuView->Enable(View_ViewPage, enable);
	toolBar->EnableTool(View_ViewPage, enable);
}

void MyFrame::OnSaveLog(wxCommandEvent& event)
{
	wxFileDialog* OpenDialog = new wxFileDialog(
		this, _("Choose name for your file"), wxEmptyString, wxEmptyString, 
		_("Text files (*.txt)|*.txt|C++ Source Files (*.cpp, *.cxx)|*.cpp;*.cxx|All files|*.*"),
		wxFD_SAVE, wxDefaultPosition);
	if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "Cancel"
	{
		wxString SavePath = OpenDialog->GetPath();
		std::ofstream * ofs = new std::ofstream(SavePath.mb_str()); // XXX LEAK!!!!
		std::clog.rdbuf(ofs->rdbuf());
		std::clog << "Goes to file." << std::endl;
	}
}


