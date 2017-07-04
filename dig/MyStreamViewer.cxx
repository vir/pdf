#include "MyStreamViewer.hpp"
#include <wx/spinctrl.h>
#include "MyFrame.hpp" // for Save()
#include "pdfdig.hpp"
#include "PdfDoc.hpp"
#include "../wxview/MyCanvas.hpp"

enum {
	ID_DEBUG_ENABLE = wxID_HIGHEST + 1,
	ID_LOG_ENABLE,
	ID_BTN_BREAK,
	ID_SCALE_1, ID_SCALE_2, ID_SCALE_4,
};

const static int ID_TOOLBAR = 501;
const static int ID_SPIN_OPLIMIT = 502;

MyStreamViewer::MyStreamViewer(wxView* view, PDF::OH& h, PDF::OH& parenth)
	: wxFrame(NULL, wxID_ANY, _T("Stream viewer"))
	, m_oh(h)
	, m_parenth(parenth)
	, m_page(NULL)
	, m_view(view)
	, m_toolBar(NULL)
	, m_canvas(NULL)
	, m_logwin(NULL)
{
	wxString title(h.id().dump().c_str(), wxConvUTF8);
	title.insert(0, _T("Stream "));
	SetTitle(title);

	m_toolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS, ID_TOOLBAR);
	m_toolBar->AddTool(wxID_SAVE, _T("Save"), wxBitmap(), _T("Save stream into text file"));
	m_toolBar->AddSeparator();
	m_toolBar->AddCheckTool(ID_LOG_ENABLE, _T("Log"), wxBitmap(), wxNullBitmap, _T("Show log window"));
	m_toolBar->AddCheckTool(ID_DEBUG_ENABLE, _T("Debug"), wxBitmap(), wxNullBitmap, _T("Show page canvas and enable debugging"));
	m_toolBar->AddSeparator();
	m_oplimitspin = new wxSpinCtrl( m_toolBar, ID_SPIN_OPLIMIT, _T(""), wxDefaultPosition, wxSize(80,wxDefaultCoord) );
	m_oplimitspin->SetRange(0, 10000);
	m_oplimitspin->SetValue(0);
	m_toolBar->AddControl(m_oplimitspin);
	m_toolBar->EnableTool(ID_SPIN_OPLIMIT, false);
	m_toolBar->AddTool(ID_BTN_BREAK, _T("Break"), wxBitmap());
	m_toolBar->EnableTool(ID_BTN_BREAK, false);
	m_toolBar->AddSeparator();
	m_toolBar->AddRadioTool(ID_SCALE_1, _T("100%"), wxBitmap());
	m_toolBar->AddRadioTool(ID_SCALE_2, _T("200%"), wxBitmap());
	m_toolBar->AddRadioTool(ID_SCALE_4, _T("400%"), wxBitmap());
	m_toolBar->Realize();

	m_split_log = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ );
	m_splitter = new wxSplitterWindow(m_split_log, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ );
	m_splitter->SetSashGravity(1.0);
	m_split_log->SetSashGravity(1.0);

	m_text = new wxTextCtrl(m_splitter, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP|wxTE_NOHIDESEL);
	m_splitter->Initialize(m_text);
	m_split_log->Initialize(m_splitter);
	DumpObject();
}

MyStreamViewer::~MyStreamViewer()
{
	delete m_canvas;
	delete m_page;
	delete m_logwin;
}

void MyStreamViewer::DumpObject()
{
	m_buf.clear();
	m_text->Clear();

	PDF::Stream * s = NULL;
	// check for array page content
	if(m_oh->type() == "Array") {
		for(unsigned int i = 0; i < m_oh.size(); ++i) {
			PDF::OH el = m_oh[i];
			el.expand();
			el.put(s);
			if(s)
				DumpObject(s);
		}
	} else {
		m_oh.put(s);
		if(s)
			DumpObject(s);
	}
	wxString txt;
	txt.Alloc(m_buf.size());
	for(size_t i = 0; i < m_buf.size(); ++i) {
		char c = m_buf[i];
		if(!is_ok(c))
			c = '?';
		txt.Append(c);
	}
	m_text->SetValue(txt);
}
void MyStreamViewer::DumpObject(PDF::Stream* s)
{
	wxASSERT(s);
	std::vector<char> buf;
	s->get_data(buf);
	m_buf.insert(m_buf.end(), buf.begin(), buf.end());
}

BEGIN_EVENT_TABLE(MyStreamViewer, wxFrame)
EVT_MENU      (wxID_SAVE,       MyStreamViewer::OnSave)
EVT_MENU      (ID_DEBUG_ENABLE, MyStreamViewer::OnDebug)
EVT_MENU      (ID_LOG_ENABLE,   MyStreamViewer::OnLog)
EVT_MENU      (ID_BTN_BREAK,    MyStreamViewer::OnBreak)
EVT_MENU_RANGE(ID_SCALE_1, ID_SCALE_4, MyStreamViewer::OnScale)
EVT_SPINCTRL  (ID_SPIN_OPLIMIT, MyStreamViewer::OnOpLimit)
END_EVENT_TABLE()

void MyStreamViewer::OnSave(wxCommandEvent& event)
{
	MyFrame* mf = static_cast<MyFrame*>(static_cast<wxAppBase*>(MyApp::GetInstance())->GetTopWindow());
	mf->OnSaveStream(event);
}

void MyStreamViewer::OnDebug(wxCommandEvent& event)
{
	if(event.IsChecked()) {
		if(ParsePage()) {
			m_canvas = new MyCanvas(m_splitter);
			m_canvas->SetPage(m_page);
			m_splitter->SplitVertically(m_text, m_canvas, 300);
			m_toolBar->EnableTool(ID_SPIN_OPLIMIT, true);
			m_page->set_operators_number_limit(m_oplimitspin->GetValue());
			m_canvas->Refresh();
			m_canvas->Update();
			UpdateLogWindow();
		} else
			m_toolBar->ToggleTool(ID_DEBUG_ENABLE, false);
	} else {
		m_splitter->Unsplit(m_canvas);
		delete m_canvas;
		m_canvas = NULL;
		delete m_page;
		m_page = NULL;
		m_toolBar->EnableTool(ID_SPIN_OPLIMIT, false);
	}
}

void MyStreamViewer::OnLog(wxCommandEvent& event)
{
	if(event.IsChecked()) {
		m_logwin = new wxTextCtrl(m_split_log, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY);
		m_split_log->SplitHorizontally(m_splitter, m_logwin);
		UpdateLogWindow();
	} else {
		m_split_log->Unsplit(m_logwin);
		delete m_logwin;
		m_logwin = NULL;
	}
}

void MyStreamViewer::OnOpLimit(wxSpinEvent& WXUNUSED(event))
{
	int lim = m_oplimitspin->GetValue();
	if(m_page) {
		m_page->set_operators_number_limit(lim);
		m_oplimitspin->SetRange(0, m_page->get_operators_count());
		if(lim)
			SelectOperator(lim);
	}
	if(m_canvas) {
		m_canvas->debug(lim > 0);
		m_canvas->Refresh();
#ifdef _DEBUG
		m_toolBar->EnableTool(ID_BTN_BREAK, true);
#endif
	}
	UpdateLogWindow();
}

void MyStreamViewer::SelectOperator(unsigned int lim)
{
	/* The multiline text controls always uses CRLF as internal EndOfLine indidcator.
	 * So we counting LF without preceding CR to get proper char index.
	 */
	size_t begin = m_page->get_operator_offset(lim - 1);
	size_t end = m_page->get_operator_offset(lim);
	begin += GetOffsetCorrection(begin);
	end += GetOffsetCorrection(end);
	m_text->SetSelection(begin, end);
}

bool MyStreamViewer::ParsePage()
{
	delete m_page;
	m_page = NULL;

	try {
//		PdfDoc * doc = static_cast<PdfDoc*>(m_view->GetDocument());
		m_page = new PDF::Page();
		m_page->debug(5);
		m_page->load(m_parenth);
		// std::clog << page->dump() << std::endl;
		return true;
	}
	catch(std::string& s) {
		std::cerr << "!Exception: " << s << std::endl;
	}
	catch(PDF::DocumentStructureException& e) {
		std::cerr << "DocumentStructureException:\n  " << e.what() << std::endl;
	}
	catch(PDF::FormatException& e) {
		std::cerr << "Format excertion:\n  " << e.what() << std::endl;
	}
	catch(std::exception& e) {
		std::cerr << "Unknown exception:\n  " << e.what() << std::endl;
	}
	catch(...) {
		std::cerr << "Unknown exception!" << std::endl;
	}
	/* reached only after exception */
	return false;
}

int MyStreamViewer::GetOffsetCorrection(unsigned int lim)
{
	size_t crcnt = 0;
#ifdef __WINDOWS__
	for(size_t i = 0; i < lim; ++i) {
		switch(m_buf[i]) {
		case '\r':
			if(i + 1 < lim && m_buf[i + 1] == '\n') {
				++i;
				break;
			}
			// else fall thrugh!
		case '\n':
			++crcnt;
			break;
		}
	}
#endif /* __WINDOWS__ */
	return crcnt;
}

void MyStreamViewer::UpdateLogWindow()
{
	if(m_logwin && m_canvas) {
		m_logwin->SetValue(wxString(m_canvas->get_page_log().c_str(), wxConvUTF8));
		m_logwin->ShowPosition(m_logwin->GetLastPosition());
	}
}

void MyStreamViewer::OnBreak(wxCommandEvent& event)
{
	if(m_canvas) {
		m_canvas->set_break();
		m_canvas->Refresh();
	}
}

void MyStreamViewer::OnScale(wxCommandEvent& event)
{
	if(! m_canvas)
		return;
	switch(event.GetId()) {
		case ID_SCALE_1: m_canvas->set_scale(1.0); break;
		case ID_SCALE_2: m_canvas->set_scale(2.0); break;
		case ID_SCALE_4: m_canvas->set_scale(4.0); break;
		default: break;
	}
	m_canvas->SetPageSize();
	m_canvas->Refresh();
}

