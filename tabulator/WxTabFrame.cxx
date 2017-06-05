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
#include "PageNumCtrl.hpp"
#include "WxTabBatchExportDlg.hpp"
#include "options16.xpm"
#include "csv16.xpm"
#include "xml16.xpm"
#ifdef _WIN32
# include "excel16.xpm"
#endif
#include "book16.xpm"
#include "thispage.xpm"


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

	MenuZoom_First,
	Zoom_100 = MenuZoom_First,
	Zoom_200,
	Zoom_300,
	Zoom_400,
	Zoom_500,
	MenuZoom_Last = Zoom_500,

	MenuGo_First,
	Go_First = MenuGo_First, Go_Prev, Go_Next, Go_Last,
	MenuGo_Last = Go_Last,

	MenuExport_First,
	Export_CSV = MenuExport_First, Export_HTML, Export_EXCEL,
	MenuExport_Last = Export_EXCEL,

	Export_Batch,

	Batch_First,
	Batch_ThisIsFirstPage = Batch_First,
	Batch_ThisIsLastPage,
	Batch_StartExport,
	Batch_Last,

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
	EVT_MENU_RANGE(MenuZoom_First,     MenuZoom_Last,     WxTabFrame::OnZoom)
	EVT_MENU_RANGE(MenuExport_First,   MenuExport_Last,   WxTabFrame::OnMenuExport)
	EVT_MENU      (Export_Batch,  WxTabFrame::OnBatchExport)
#if 0
	EVT_SPINCTRL  (ID_SPIN_OPLIMIT, WxTabFrame::OnOplimitSpinCtrl)
#endif
	EVT_TEXT_ENTER(ID_PAGENUM,    WxTabFrame::OnPageNumChanged)
	EVT_MENU_RANGE(Batch_First, Batch_Last, WxTabFrame::OnBatchCmd)
END_EVENT_TABLE()

WxTabFrame::WxTabFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
	, m_batch_export(NULL)
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

	/* Add Menu */
	wxMenu * menuDocument = new wxMenu;
	menuDocument->Append(File_Open, _T("&Open\tCtrl-O"), _T("Open PDF file"));
	menuDocument->AppendSeparator();
	menuDocument->Append(File_ShowTabulatorOptions, _T("Tabulator &Options\tAlt-O"), _T("Show Tabulator options dialog"));
	menuDocument->AppendSeparator();
	menuDocument->Append(File_About, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	menuDocument->Append(File_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

	wxMenu * menuView = new wxMenu;
	menuView->AppendRadioItem(Rotate_0, _T("No rotation\tF1"));
	menuView->AppendRadioItem(Rotate_90, _T("Rotate &90 degree\tF2"));
	menuView->AppendRadioItem(Rotate_180, _T("Rotate &180 degree\tF3"));
	menuView->AppendRadioItem(Rotate_270, _T("Rotate &270 degrees\tF4"));

	menuView->AppendRadioItem(Zoom_100, _T("Scale &100%\tz"));
	menuView->AppendRadioItem(Zoom_200, _T("Scale &200%\tx"));
	menuView->AppendRadioItem(Zoom_300, _T("Scale &300%\tc"));
	menuView->AppendRadioItem(Zoom_400, _T("Scale &400%\tv"));
	menuView->AppendRadioItem(Zoom_500, _T("Scale &500%\tb"));

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
	menuExport->AppendSeparator();
	menuExport->Append(Export_Batch, _T("Batch export..."));

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuDocument, _T("&Document"));
	menuBar->Append(menuView, _T("&View"));
	menuBar->Append(menuGo, _T("&Go"));
	menuBar->Append(menuExport, _T("&Export"));

	SetMenuBar(menuBar);

	/* Add Toolbar */
	wxAuiToolBar *toolBar;
	toolBar	= new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE /*| wxAUI_TB_TEXT | wxAUI_TB_HORZ_TEXT*/);
	toolBar->SetToolBitmapSize(wxSize(16,16));

	// Populate Document Toolbar
	toolBar->AddTool(wxID_OPEN, _T("Open"), wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, wxSize(16,16)), _T("Open file"));
	toolBar->AddTool(Go_Prev, _T("<"), wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_TOOLBAR, wxSize(16,16)), _T("Go to the previous page"));
	m_pagenum = new PageNumCtrl(toolBar, ID_PAGENUM, _T(""));
	if(theDocument) {
		m_pagenum->SetRange(1, theDocument->GetPagesNum());
		m_pagenum->SetValue(theDocument->GetPageNum());
	}
	toolBar->AddControl(m_pagenum);
	toolBar->AddTool(Go_Next, _T(">"), wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR, wxSize(16,16)), _T("Go to the next page"));
	toolBar->Realize();
	m_mgr.AddPane(toolBar, wxAuiPaneInfo().
		Name(wxT("Toolbar1")).Caption(wxT("Document Tools")).ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));

	// one more toolbar
	toolBar	= new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_TEXT | wxAUI_TB_HORZ_TEXT);
	toolBar->SetToolBitmapSize(wxSize(16,16));

	// Populate Table Toolbar
	toolBar->AddTool(File_ShowTabulatorOptions, _T("Options"), wxBitmap(options16_xpm), _T("Show Tabulator options dialog"));
	toolBar->AddTool(Export_CSV, _T("CSV"), wxBitmap(csv16_xpm), _T("Export current table as CSV"));
	toolBar->AddTool(Export_HTML, _T("XML"), wxBitmap(xml16_xpm), _T("Export current table as XML table"));
#ifdef _WIN32
	toolBar->AddTool(Export_EXCEL, _T("Excel"), wxBitmap(excel16_xpm), _T("Export current table to Microsoft Excel (Open empty sheet if already running!!!)"));
#endif
	toolBar->AddSeparator();
	toolBar->AddTool(Export_Batch, _T("Batch"), wxBitmap(book16_xpm), _T("Open batch export dialog"));

	toolBar->Realize();
	m_mgr.AddPane(toolBar, wxAuiPaneInfo().
		Name(wxT("Toolbar2")).Caption(wxT("Second Toolbar")).ToolbarPane().Top().Position(1).
		LeftDockable(false).RightDockable(false));

	// batch export toolbar
	m_batch_export = new BatchExport(this);
	toolBar = m_batch_export->CreateToolBar();
	toolBar->Realize();
	m_mgr.AddPane(toolBar, wxAuiPaneInfo().Name(wxT("BatchExport")).Caption(wxT("Batch Export")).ToolbarPane().Top().Row(1).LeftDockable(false).RightDockable(false));

	/* Add Status Bar */
	CreateStatusBar(1);
	SetStatusText(_T("Welcome to wxWidgets!"));

	/* And finally our main widget */
	m_canvas = new WxTabCanvas( this );
	m_canvas->SetScrollbars( 10, 10, 100, 240 );
	//m_canvas->SetScrollRate(10, 10);
	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().Name(wxT("canvas")).CenterPane().PaneBorder(false));

	// "commit" all changes made to wxAuiManager
	m_mgr.Update();
}

WxTabFrame::~WxTabFrame()
{
	delete m_batch_export;
	delete theDocument;
	theDocument = NULL;
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
		theDocument->SetErrorHandler(this);
		if(theDocument->Open(path)) {
			SetTitle(of.GetFilename());
			m_pagenum->SetValue(theDocument->GetPageNum());
			m_canvas->SetScrolledPageSize();
			m_canvas->Refresh();
		}
		else
		{
			delete theDocument;
			theDocument = NULL;
		}
	}
}

void WxTabFrame::OnRotate(wxCommandEvent& event)
{
	int rot = event.GetId() - MenuRotate_First;
	theDocument->Rotate(rot);
	m_canvas->SetScrolledPageSize();
	m_canvas->Refresh();
}

void WxTabFrame::OnZoom(wxCommandEvent& event)
{
	int scale = 1 + event.GetId() - MenuZoom_First;
	m_canvas->set_scale(scale);
	theDocument->Scale(100 * scale);
	m_canvas->SetScrolledPageSize();
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
			std::cerr << "Unhandled Go_* Event! " << event.GetId() << std::endl;
			break;
	}
	theDocument->LoadPage(pn);
	m_pagenum->SetValue(pn);
	m_canvas->SetScrolledPageSize();
	m_canvas->Refresh();
}

void WxTabFrame::OnPageNumChanged(wxCommandEvent &event)
{
	int v = m_pagenum->GetValue();
	if(v > theDocument->GetPagesNum()) {
		v = theDocument->GetPagesNum();
		m_pagenum->SetValue(v);
	}
	
	theDocument->LoadPage(v);
	std::clog << "Page switched to " << v << std::endl;
	m_pagenum->SetValue(v);
	m_canvas->SetScrolledPageSize();
	m_canvas->Refresh();
}

void WxTabFrame::OnBatchCmd(wxCommandEvent & event)
{
	if(m_batch_export)
		m_batch_export->OnCmd(event);
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
	if(exporter && theDocument && theDocument->export_ok()) {
		std::string fn(theDocument->GetName().utf8_str());
		exporter->page_begin(fn, theDocument->GetPageNum());
		theDocument->tabulator.output(exporter); // XXX
		exporter->page_end();
	}
	delete exporter;
}

void WxTabFrame::OnBatchExport(wxCommandEvent &event)
{
	if(theDocument) {
		WxTabBatchExportDialog dlg(this, theDocument);
		dlg.SetCurPage(theDocument->GetPageNum());
		dlg.ShowModal();
	}
}

void WxTabFrame::ReportError(const char * prefix, const char * msg)
{
	wxMessageBox(wxString(msg, wxConvUTF8), wxString(prefix, wxConvUTF8), wxOK|wxCENTRE, this);
}

inline void BatchExport::OnCmd(wxCommandEvent & event)
{
	int pn = theDocument->GetPageNum();
	switch(event.GetId()) {
	case Batch_ThisIsFirstPage:
		m_firstpage->SetValue(pn);
		break;
	case Batch_ThisIsLastPage:
		m_lastpage->SetValue(pn);
		break;
	case Batch_StartExport:
		if(theDocument) {
			WxTabBatchExportDialog dlg(m_parent, theDocument);
			dlg.SetPages(m_firstpage->GetValue(), m_lastpage->GetValue(), m_pages_in_row->GetValue());
			dlg.ShowModal();
		}
		break;
	default:
		std::cerr << "Unhandled Batch_* Event! " << event.GetId() << std::endl;
		break;
	}
}

wxAuiToolBar* BatchExport::CreateToolBar()
{
	wxAuiToolBar* toolBar = new wxAuiToolBar(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_TEXT | wxAUI_TB_HORZ_TEXT);
	toolBar->SetToolBitmapSize(wxSize(16, 16));
	m_firstpage = new PageNumCtrl(toolBar, ID_PAGENUM, _T(""), wxDefaultPosition, wxSize(50, -1));
	m_lastpage = new PageNumCtrl(toolBar, ID_PAGENUM, _T(""), wxDefaultPosition, wxSize(50, -1));
	if(theDocument) {
		m_firstpage->SetRange(1, theDocument->GetPagesNum());
		m_lastpage->SetRange(1, theDocument->GetPagesNum());
	}
	toolBar->AddLabel(Batch_ThisIsFirstPage, _T("First"));
	toolBar->AddTool(Batch_ThisIsFirstPage, _T(""), wxBitmap(thispage_xpm), _T("Use current page as first page"));
	toolBar->AddControl(m_firstpage);
	toolBar->AddLabel(Batch_ThisIsLastPage, _T("Last"));
	toolBar->AddTool(Batch_ThisIsLastPage, _T(""), wxBitmap(thispage_xpm), _T("Use current page as last page"));
	toolBar->AddControl(m_lastpage);
	m_pages_in_row = new wxSpinCtrl(toolBar, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(65, -1));
	m_pages_in_row->SetMin(1);
	if(theDocument)
		m_pages_in_row->SetMax(theDocument->GetPagesNum());
	toolBar->AddLabel(Batch_ThisIsLastPage, _T("In row"));
	toolBar->AddControl(m_pages_in_row);

	toolBar->AddSeparator();
	
	toolBar->AddTool(Export_CSV, wxEmptyString, wxBitmap(csv16_xpm), _T("CSV"), wxITEM_RADIO);
	toolBar->AddTool(Export_HTML, wxEmptyString, wxBitmap(xml16_xpm), _T("XML"), wxITEM_RADIO);
#ifdef _WIN32
	toolBar->AddTool(Export_EXCEL, wxEmptyString, wxBitmap(excel16_xpm), _T("Microsoft Excel"), wxITEM_RADIO);
#endif
	toolBar->AddSeparator();

	toolBar->AddTool(Batch_StartExport, _T("Start"), wxBitmap(), _T("Start batch export"));
	return toolBar;
}
