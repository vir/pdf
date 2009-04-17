// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif
#include <wx/artprov.h>
#include "WxTabFrame.hpp"
#include "WxTabCanvas.hpp"
#include "WxTabDocument.hpp"
#include "WxTabTabulator.hpp"
#include "Tabulator_Exporter.hpp"
#include "excel16.xpm"

class PageNumCtrl:public wxTextCtrl
{
	private:
		long num;
		long min, max;
	public:
		PageNumCtrl(wxWindow* parent, wxWindowID id, const wxString& value = _T(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTE_CENTRE|wxTE_PROCESS_ENTER, const wxString& name = wxTextCtrlNameStr):wxTextCtrl(parent, id, value, pos, size, style, wxDefaultValidator, name),num(0),min(0),max(10000)
		{
		}
		void OnChar( wxKeyEvent & event )
		{
#if 1
			wxString backup = wxTextCtrl::GetValue();
			long tmp;
			if( event.GetKeyCode() < 32 || event.GetKeyCode() == 127 || event.GetKeyCode() > 256) event.Skip();
			if( !isdigit( event.GetKeyCode() ) ) return;
//			EmulateKeyPress(event); // causes infinite recursion on win32
			event.Skip();
			wxString newval = wxTextCtrl::GetValue();
			if( !newval.ToLong(&tmp) || tmp<min || tmp>max ) {
				wxTextCtrl::SetValue(backup);
				wxBell();
			} else {
				num = tmp;
			}
#else
			event.Skip();
#endif
		}
		void OnSetFocus( wxFocusEvent & event )
		{
			wxTextCtrl::SetSelection(-1, -1);
		}
		void SetRange(int r1, int r2)
		{
			min = r1;
			max = r2;
		}
		void SetValue(int i)
		{
			num = i;
			wxTextCtrl::SetValue(wxString::Format(_T("%d"), i));
		}
		int GetValue()
		{
			long tmp;
			wxString newval = wxTextCtrl::GetValue();
			if( newval.ToLong(&tmp) || tmp<min || tmp>max ) {
				num = tmp;
			}
			return num;
		}
    DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE(PageNumCtrl, wxTextCtrl)
	EVT_CHAR      (PageNumCtrl::OnChar)
	EVT_SET_FOCUS (PageNumCtrl::OnSetFocus)
END_EVENT_TABLE()

enum {
	File_Open = wxID_OPEN,
	File_Quit = wxID_EXIT,
	File_About = wxID_ABOUT,
	File_ShowTabulatorOptions = wxID_HIGHEST,

	MenuRotate_First,
	Rotate_0 = MenuRotate_First,
	Rotate_90,
	Rotate_180,
	Rotate_270,
	MenuRotate_Last = Rotate_270,

	MenuGo_First,
	Go_First = MenuGo_First, Go_Prev, Go_Next, Go_Last,
	MenuGo_Last = Go_Last,

	MenuExport_First,
	Export_CSV = MenuExport_First, Export_HTML, Export_EXCEL,
	MenuExport_Last = Export_EXCEL,
};

const int ID_TOOLBAR = 500;
const int ID_SPIN_OPLIMIT = 511;
const int ID_PAGENUM = 512;

BEGIN_EVENT_TABLE(WxTabFrame, wxFrame)
	EVT_MENU      (File_Quit,     WxTabFrame::OnQuit)
	EVT_MENU      (File_About,    WxTabFrame::OnAbout)
	EVT_MENU      (File_Open,     WxTabFrame::OnDocumentOpen)
	EVT_MENU      (File_ShowTabulatorOptions,    WxTabFrame::OnShowTabulatorOptions)
	EVT_MENU_RANGE(MenuGo_First,  MenuGo_Last,   WxTabFrame::OnMenuGo)
	EVT_MENU_RANGE(MenuRotate_First,   MenuRotate_Last,   WxTabFrame::OnRotate)
	EVT_MENU_RANGE(MenuExport_First,   MenuExport_Last,   WxTabFrame::OnMenuExport)
#if 0
	EVT_SPINCTRL  (ID_SPIN_OPLIMIT, WxTabFrame::OnOplimitSpinCtrl)
#endif
	EVT_TEXT_ENTER(ID_PAGENUM,    WxTabFrame::OnPageNumChanged)
END_EVENT_TABLE()

WxTabFrame::WxTabFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
//	, m_mgr(this, wxAUI_MGR_DEFAULT | wxAUI_MGR_ALLOW_FLOATING)
{
	m_mgr.SetManagedWindow(this);
	SetMinSize(wxSize(400,300));
	m_mgr.SetFlags(m_mgr.GetFlags()
		| wxAUI_MGR_ALLOW_FLOATING
		| wxAUI_MGR_TRANSPARENT_DRAG
		| wxAUI_MGR_HINT_FADE
		| wxAUI_MGR_TRANSPARENT_HINT
	);

//	m_oplimitspin = NULL;
//	m_pagenum = NULL;

	/* Add Menu */
	wxMenu * menuDocument = new wxMenu;
	menuDocument->Append(File_Open, _T("&Open\tCtrl-O"), _T("Open PDF file"));
	menuDocument->AppendSeparator();
	menuDocument->Append(File_ShowTabulatorOptions, _T("Tabulator &Options\tAlt-O"), _T("Show Tabulator options dialog"));
	menuDocument->AppendSeparator();
	menuDocument->Append(File_About, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	menuDocument->Append(File_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

	wxMenu * menuRotate = new wxMenu;
	menuRotate->AppendRadioItem(Rotate_0, _T("No rotation\tF1"));
	menuRotate->AppendRadioItem(Rotate_90, _T("Rotate &90 degree\tF2"));
	menuRotate->AppendRadioItem(Rotate_180, _T("Rotate &180 degree\tF3"));
	menuRotate->AppendRadioItem(Rotate_270, _T("Rotate &270 degrees\tF4"));

	wxMenu * menuGo = new wxMenu;
	menuGo->Append(Go_First, _T("First page\tHome"));
	menuGo->Append(Go_Last, _T("Last page\tEnd"));
	menuGo->Append(Go_Prev, _T("Previous page\tPgUp"));
	menuGo->Append(Go_Next, _T("Next page\tPgDn"));

	wxMenu * menuExport = new wxMenu;
	menuExport->Append(Export_CSV, _T("Export as CSV"));
	menuExport->Append(Export_HTML, _T("Export HTML table"));
#ifdef _WIN32
	menuExport->Append(Export_EXCEL, _T("Export to MS Excel"));
#endif

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuDocument, _T("&Document"));
	menuBar->Append(menuRotate, _T("&Rotate"));
	menuBar->Append(menuGo, _T("&Go"));
	menuBar->Append(menuExport, _T("&Export"));

	SetMenuBar(menuBar);

	/* Add Toolbar */
	wxAuiToolBar *toolBar;
//	toolBar	= new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER | wxTB_HORZ_TEXT | wxTB_NOICONS);
	toolBar	= new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_TEXT | wxAUI_TB_HORZ_TEXT);
	toolBar->SetToolBitmapSize(wxSize(16,16));
//    wxBitmap tb4_bmp1 = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16));

	// Populate Document Toolbar
	toolBar->AddTool(wxID_OPEN, _T("Open"), wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, wxSize(16,16)), _T("Open file"));
//	toolBar->AddTool(Go_First, _T("<<"), wxArtProvider::GetBitmap(wxART_GO_BEGIN, wxART_TOOLBAR, wxSize(16,16)), _T("Go to the first page"));
	toolBar->AddTool(Go_Prev, _T("<"), wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_TOOLBAR, wxSize(16,16)), _T("Go to the previous page"));

	m_pagenum = new PageNumCtrl(toolBar, ID_PAGENUM, _T(""));
	if(theDocument) {
		m_pagenum->SetRange(1, theDocument->GetPagesNum());
		m_pagenum->SetValue(theDocument->GetPageNum());
	}
	toolBar->AddControl(m_pagenum);

	toolBar->AddTool(Go_Next, _T(">"), wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR, wxSize(16,16)), _T("Go to the next page"));
//	toolBar->AddTool(Go_Last, _T(">>"), wxArtProvider::GetBitmap(wxART_GO_END, wxART_TOOLBAR, wxSize(16,16)), _T("Go to the last page"));

//	toolBar->AddTool(wxID_EXIT, _T("Exit"), wxBitmap(), _T("Exit application"));
#if 0 // Debugging option!
	m_oplimitspin = new wxSpinCtrl( toolBar, ID_SPIN_OPLIMIT, _T("")/*, wxDefaultPosition, wxSize(40,wxDefaultCoord)*/ );
	m_oplimitspin->SetRange(0, 10000);
	m_oplimitspin->SetValue(0);
	toolBar->AddControl(m_oplimitspin);
#endif
	toolBar->Realize();
	m_mgr.AddPane(toolBar, wxAuiPaneInfo().
		Name(wxT("Toolbar1")).Caption(wxT("Document Tools")).ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));

	// one more toolbar
	toolBar	= new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_TEXT | wxAUI_TB_HORZ_TEXT);
	toolBar->SetToolBitmapSize(wxSize(16,16));

	// 	Populate Table Toolbar
	toolBar->AddTool(File_ShowTabulatorOptions, _T("Options"), wxBitmap(), _T("Show Tabulator options dialog"));
	toolBar->AddTool(Export_CSV, _T("CSV"), wxBitmap(), _T("Export current table as CSV to stdout"));
	toolBar->AddTool(Export_HTML, _T("HTML"), wxBitmap(), _T("Export current table as HTML table to stdout"));
#ifdef _WIN32
	toolBar->AddTool(Export_EXCEL, _T("Excel"), wxBitmap(excel16_xpm), _T("Export current table to Microsoft Excel (Open empty sheet if already running!!!)"));
#endif

	toolBar->Realize();
	m_mgr.AddPane(toolBar, wxAuiPaneInfo().
		Name(wxT("Toolbar2")).Caption(wxT("Second Toolbar")).ToolbarPane().Top().Position(1).
		LeftDockable(false).RightDockable(false));

	/* Add Status Bar */
	CreateStatusBar(2);
	SetStatusText(_T("Welcome to wxWidgets!"));

	/* And finally owr main widget */
	m_canvas = new WxTabCanvas( this );
	m_canvas->SetScrollbars( 10, 10, 100, 240 );
	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().Name(wxT("canvas")).CenterPane().PaneBorder(false));

	// "commit" all changes made to wxAuiManager
	m_mgr.Update();
}

WxTabFrame::~WxTabFrame()
{
    m_mgr.UnInit();
}

void WxTabFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void WxTabFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxString msg;
	msg.Printf(
		wxT("This is my humble attempt to parse PDF page.\n")
		wxT("Sorry for bugs.")
	);
	wxMessageBox(msg, _T("About this buggy application"), wxOK | wxICON_INFORMATION, this);
}

void WxTabFrame::OnDocumentOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog of(this, wxT("Open PDF document"),
		wxEmptyString, wxEmptyString,
		_("PDF files (*.pdf)|*.pdf|All Files (*.*)|*"),
		wxFD_OPEN, wxDefaultPosition);
	if (of.ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
	{
		wxString path = of.GetPath();
		if(!theDocument)
			theDocument = new WxTabDocument();
 		theDocument->Open(path);
		SetTitle(of.GetFilename());
		m_pagenum->SetValue(theDocument->GetPageNum());
		m_canvas->Refresh();
	}
}

void WxTabFrame::OnRotate(wxCommandEvent& event)
{
	int rot = event.GetId() - MenuRotate_First;
	theDocument->Rotate(rot);
	m_canvas->Refresh();
}

void WxTabFrame::PrepareDC(wxDC& dc)
{
#if 0
	dc.SetLogicalOrigin( m_xLogicalOrigin, m_yLogicalOrigin );
	dc.SetAxisOrientation( !m_xAxisReversed, m_yAxisReversed );
	dc.SetUserScale( m_xUserScale, m_yUserScale );
	dc.SetMapMode( m_mapMode );
#endif
}

void WxTabFrame::OnOplimitSpinCtrl(wxSpinEvent& event)
{
#if 0
	if(m_oplimitspin) {
		PDF::Page * p = theDocument->GetPageObject();
		p->set_operators_number_limit(m_oplimitspin->GetValue());
		m_canvas->Refresh();
	}
#endif
}

void WxTabFrame::OnMenuGo(wxCommandEvent &event)
{
	int pn = theDocument->GetPageNum();
	switch(event.GetId()) {
		case Go_First: pn = 1; break;
		case Go_Last: pn = theDocument->GetPagesNum(); break;
		case Go_Prev: if(pn >1) pn--; break;
		case Go_Next: if(pn < theDocument->GetPagesNum()) pn++; break;
		default:
			std::cerr << "Unhendled Go_* Event! " << event.GetId() << std::endl;
			break;
	}
	theDocument->LoadPage(pn);
	m_pagenum->SetValue(pn);
	m_canvas->Refresh();
}

void WxTabFrame::OnPageNumChanged(wxCommandEvent &event)
{
	int v = m_pagenum->GetValue();
	theDocument->LoadPage(v);
	std::clog << "Page switched to " << v << std::endl;
	m_pagenum->SetValue(v);
	m_canvas->Refresh();
}

void WxTabFrame::OnShowTabulatorOptions(wxCommandEvent& event)
{
	if(theDocument) {
		theDocument->tabulator.ShowOptionsDialog(this);
		theDocument->tabulator.full_process(theDocument->GetPageObject()); // XXX
		m_canvas->Refresh();
	}
}

void WxTabFrame::OnMenuExport(wxCommandEvent &event)
{
	Tabulator::Table::Exporter * exporter = NULL;
	switch(event.GetId()) {
		case Export_CSV:
			exporter = new ExporterCSV(std::cout);
			break;
		case Export_HTML:
			exporter = new ExporterHTML(std::cout);
			break;
#ifdef _WIN32
		case Export_EXCEL:
			{
				ExporterExcel * e = new ExporterExcel();
				if(!e->get_active()) {
					e->start_new();
					e->add_workbook();
				}
				e->set_visible(true);
				exporter = e;
			}
			break;
#endif
		default:
			std::cerr << "Unimplemented export format" << std::endl;
			break;
	}
	if(exporter && theDocument->export_ok()) {
		exporter->page_begin("wxTab document", theDocument->GetPageNum());
		theDocument->tabulator.output(exporter); // XXX
		exporter->page_end();
	}
	delete exporter;
}



