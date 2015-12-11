#include "MyStreamViewer.hpp"
#include <wx/spinctrl.h>
#include "MyFrame.hpp" // for Save()
#include "pdfdig.hpp"
#include "PdfDoc.hpp"
#include "../wxview/MyCanvas.hpp"

enum {
	ID_DEBUG_ENABLE = wxID_HIGHEST + 1,
};

const static int ID_TOOLBAR = 501;
const static int ID_SPIN_OPLIMIT = 502;

MyStreamViewer::MyStreamViewer(wxView* view, PDF::OH& h, PDF::OH& parenth)
	: wxFrame(NULL, wxID_ANY, _T("Stream viewer"))
	, m_oh(h)
	, m_parenth(parenth)
	, m_view(view)
	, m_toolBar(NULL)
	, m_page(NULL)
	, m_canvas(NULL)
{
	wxString title(h.id().dump().c_str(), wxConvUTF8);
	title.insert(0, _T("Stream "));
	SetTitle(title);

	m_toolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS, ID_TOOLBAR);
	m_toolBar->AddTool(wxID_SAVE, _T("Save"), wxBitmap(), _T("Save stream into text file"));
	m_toolBar->AddSeparator();
	m_toolBar->AddCheckTool(ID_DEBUG_ENABLE, _T("Debug"), wxBitmap(), wxNullBitmap, _T("Save stream object content"));
	m_toolBar->AddSeparator();
	m_oplimitspin = new wxSpinCtrl( m_toolBar, ID_SPIN_OPLIMIT, _T(""), wxDefaultPosition, wxSize(80,wxDefaultCoord) );
	m_oplimitspin->SetRange(0, 10000);
	m_oplimitspin->SetValue(0);
	m_toolBar->AddControl(m_oplimitspin);
	m_toolBar->EnableTool(ID_SPIN_OPLIMIT, false);
	m_toolBar->Realize();

	m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ );
	m_splitter->SetSashGravity(1.0);

	m_text = new wxTextCtrl(m_splitter, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP|wxTE_NOHIDESEL);
	m_splitter->Initialize(m_text);
	DumpObject();
}

void MyStreamViewer::DumpObject()
{
	PDF::Stream * s = NULL;
	m_oh.put(s);
	wxASSERT(s);
	s->get_data(m_buf);
	wxString txt;
	txt.Alloc(m_buf.size());
	for(size_t i = 0; i < m_buf.size(); ++i) {
		char c = m_buf[i];
		if(! is_ok(c))
			c = '?';
		txt.Append(c);
	}
	m_text->SetValue(txt);
}

BEGIN_EVENT_TABLE(MyStreamViewer, wxFrame)
EVT_MENU      (wxID_SAVE,       MyStreamViewer::OnSave)
EVT_MENU      (ID_DEBUG_ENABLE, MyStreamViewer::OnDebug)
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
	}
}

void MyStreamViewer::SelectOperator(unsigned int lim)
{
	/* The multiline text controls always uses CRLF as internal EndOfLine indidcator.
	 * So we counting LF without preceding CR to get proper char index.
	 */
	size_t begin = m_page->get_operator_offset(lim - 1);
	size_t end = m_page->get_operator_offset(lim);
	begin += CountMissedCRChars(begin);
	end += CountMissedCRChars(end);
	m_text->SetSelection(begin, end);
}

bool MyStreamViewer::ParsePage()
{
	delete m_page;
	m_page = NULL;

	try {
		PdfDoc * doc = static_cast<PdfDoc*>(m_view->GetDocument());
		m_page = new PDF::Page();
		m_page->debug(5);
		m_page->load(m_parenth);
		// std::clog << page->dump() << std::endl;
		return true;
	}
	catch(std::string s) {
		std::cerr << "!Exception: " << s << std::endl;
	}
	catch(PDF::DocumentStructureException e) {
		std::cerr << "DocumentStructureException:\n  " << e.what() << std::endl;
	}
	catch(PDF::FormatException e) {
		std::cerr << "Format excertion:\n  " << e.what() << std::endl;
	}
	catch(std::exception e) {
		std::cerr << "Unknown exception:\n  " << e.what() << std::endl;
	}
	catch(...) {
		std::cerr << "Unknown exception!" << std::endl;
	}
	/* reached only after exception */
	return false;
}

unsigned int MyStreamViewer::CountMissedCRChars(unsigned int lim)
{
	size_t crcnt = 0;
	for(size_t i = 0; i < lim; ++i)
		if((m_buf[i] == '\n' && (i == 0 || m_buf[i - 1] != '\r'))
			|| (m_buf[i] == '\r' && (i >= lim -1 || m_buf[i + 1] != '\n')))
			++crcnt;
	return crcnt;
}

