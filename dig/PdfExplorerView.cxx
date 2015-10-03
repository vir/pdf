#include "wx/wx.h"
#include "PdfExplorerView.hpp"
#include "pdfdig.hpp"
#include <PDF.hpp>
#include "PdfDoc.hpp"
#include "MyStreamViewer.hpp"

IMPLEMENT_DYNAMIC_CLASS(PdfExplorerView, wxView)

BEGIN_EVENT_TABLE(PdfExplorerView, wxView)
//	EVT_MENU(DOODLE_CUT, PdfExplorerView::OnCut)
END_EVENT_TABLE()


PdfExplorerView::PdfExplorerView()
	: m_splitter(NULL), m_tree(NULL), m_right(NULL), m_frame(NULL)
	, m_stream_handle(NULL), m_stream_parent_handle(NULL)
{
	m_mainframe = static_cast<MyApp*>(wxTheApp)->GetMainFrame();
	//// Make a child frame
	wxDocMDIChildFrame *subframe = new wxDocMDIChildFrame(GetDocument(), this, m_mainframe, wxID_ANY, _T("Child Frame"), wxPoint(10, 10), wxSize(300, 300), wxDEFAULT_FRAME_STYLE);
#ifdef __WXMSW__
	//subframe->SetIcon(wxString(isCanvas ? _T("chrt_icn") : _T("notepad_icn")));
#endif
	subframe->Centre(wxBOTH);
	m_frame = subframe;
	m_frame->Show(true);
	SetFrame(m_frame);
	m_splitter = new wxSplitterWindow(m_frame, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ );
	m_splitter->SetSashGravity(1.0);

	m_tree = new MyTree(this, m_splitter, this);
	m_right = new wxTextCtrl(m_splitter, wxID_ANY, _T("second text"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP);

	subframe->Maximize();
	m_splitter->SplitVertically(m_tree, m_right, 300);
}

PdfExplorerView::~PdfExplorerView()
{
	delete m_stream_handle;
	delete m_stream_parent_handle;
	delete m_right;
	delete m_tree;
	delete m_splitter;
	delete m_frame;
}

// What to do when a view is created.
bool PdfExplorerView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
	wxTrace("PdfExplorerView::OnCreate");
	Activate(true);
	return true;
}

// Used for default print/preview as well as drawing on the screen.
void PdfExplorerView::OnDraw(wxDC *dc)
{
	wxTrace("PdfExplorerView::OnDraw");
}

void PdfExplorerView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint))
{
	wxTrace("PdfExplorerView::OnUpdate");
	m_tree->Update();
}

// Clean up windows used for displaying the view.
bool PdfExplorerView::OnClose(bool deleteWindow)
{
	wxTrace("PdfExplorerView::OnClose");
	return true;
}

void PdfExplorerView::SelectedNothing()
{
	PdfDoc * doc = static_cast<PdfDoc *>(GetDocument());
	m_right->SetValue(doc->get_file_brief());
	m_mainframe->ViewStreamEnable(false);
	delete m_stream_handle;
	m_stream_handle = NULL;
	delete m_stream_parent_handle;
	m_stream_parent_handle = NULL;
}

void PdfExplorerView::SelectedObject(PDF::OH h, PDF::OH parent)
{
	wxString s(h->dump().c_str(), wxConvUTF8);
	m_right->SetValue(s);
	bool is_a_stream = h->type() == "Stream";
	m_mainframe->ViewStreamEnable(is_a_stream);
	delete m_stream_handle;
	m_stream_handle = NULL;
	delete m_stream_parent_handle;
	m_stream_parent_handle = NULL;
	if(is_a_stream) {
		m_right->AppendText(_T("\n\n*** Hit F3 to view stream data or F2 to save it! ***\n"));
		m_stream_handle = new PDF::OH(h);
		m_stream_parent_handle = new PDF::OH(parent);
	}
}

void PdfExplorerView::ViewStreamData()
{
	wxASSERT(m_stream_handle);
	wxFrame * f = new MyStreamViewer(this, *m_stream_handle, *m_stream_parent_handle);
	f->Show();
}

void PdfExplorerView::SaveStreamData(std::ostream& ostr)
{
	if(! m_stream_handle)
		return;
	PDF::Stream * s = NULL;
	m_stream_handle->put(s);
	wxASSERT(s);
	std::vector<char> buf;
	s->get_data(buf);
	ostr.write(&*buf.begin(), buf.size());
}



