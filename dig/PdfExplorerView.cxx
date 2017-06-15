#include "wx/wx.h"
#include "PdfExplorerView.hpp"
#include "pdfdig.hpp"
#include <PDF.hpp>
#include "PdfDoc.hpp"
#include "MyStreamViewer.hpp"

wxIMPLEMENT_DYNAMIC_CLASS(PdfExplorerView, wxView)

BEGIN_EVENT_TABLE(PdfExplorerView, wxView)
//	EVT_MENU(DOODLE_CUT, PdfExplorerView::OnCut)
END_EVENT_TABLE()


PdfExplorerView::PdfExplorerView()
	: m_splitter(NULL), m_tree(NULL), m_right(NULL), m_frame(NULL)
	, m_stream_handle(NULL), m_page_handle(NULL)
{
	m_mainframe = static_cast<MyApp*>(wxTheApp)->GetMainFrame();
}

PdfExplorerView::~PdfExplorerView()
{
	delete m_stream_handle;
	delete m_page_handle;
	delete m_right;
	delete m_tree;
	delete m_splitter;
	delete m_frame;
}

// What to do when a view is created.
bool PdfExplorerView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
	wxTrace("PdfExplorerView::OnCreate");

	//// Make a child frame
	wxFrame* subframe = subframe = new wxDocMDIChildFrame(doc, this, wxStaticCast(wxGetApp().GetTopWindow(), wxDocMDIParentFrame),
		wxID_ANY, "Child Frame", wxDefaultPosition, wxSize(300, 300));
	wxASSERT(subframe == GetFrame());
	subframe->Centre(wxBOTH);
	m_frame = subframe;
	//SetFrame(m_frame);
	m_splitter = new wxSplitterWindow(m_frame, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */);
	m_splitter->SetSashGravity(1.0);

	m_tree = new MyTree(this, m_splitter, this);
	m_right = new wxTextCtrl(m_splitter, wxID_ANY, _T("second text"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);

	subframe->Maximize();
	m_splitter->SplitVertically(m_tree, m_right, 300);


	m_frame->Show(true);
	//Activate(true);
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
	if (!wxView::OnClose(deleteWindow))
		return false;

	Activate(false);
	if (deleteWindow)
	{
		GetFrame()->Destroy();
		SetFrame(NULL);
	}
	return true;
}

void PdfExplorerView::SelectedNothing()
{
	PdfDoc * doc = static_cast<PdfDoc *>(GetDocument());
	m_right->SetValue(doc->get_file_brief());
	m_mainframe->ViewStreamEnable(false);
	delete m_stream_handle;
	m_stream_handle = NULL;
	delete m_page_handle;
	m_page_handle = NULL;
}

void PdfExplorerView::SelectedObject(PDF::OH h, PDF::OH parent)
{
	wxString s(h->dump().c_str(), wxConvUTF8);
	m_right->SetValue(s);

	bool is_a_stream = h->type() == "Stream";

	m_mainframe->ViewStreamEnable(is_a_stream);

	delete m_stream_handle;
	m_stream_handle = NULL;
	delete m_page_handle;
	m_page_handle = NULL;

	if(is_a_stream) {
		m_right->AppendText(_T("\n\n*** Hit F3 to view stream data or F2 to save it! ***\n"));
		m_stream_handle = new PDF::OH(h);
		return;
	}

	bool is_a_page = false;
	PDF::Dictionary* d = h.cast<PDF::Dictionary*>();
	if(d) {
		PDF::Name* t = dynamic_cast<PDF::Name*>(d->find("Type"));
		is_a_page = t && t->value() == "Page";
	}
	m_mainframe->ViewPageEnable(is_a_page);
	if(is_a_page)
	{
		m_page_handle = new PDF::OH(h);
		return;
	}
}

void PdfExplorerView::ViewStreamData()
{
	wxASSERT(m_stream_handle);
	PDF::OH* m_stream_parent_handle = m_stream_handle; // XXX temp hack to fix build!!!
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

void PdfExplorerView::ViewPage()
{
	wxASSERT(m_page_handle);
	PDF::OH content_stream = m_page_handle->find("Contents");
	wxFrame * f = new MyStreamViewer(this, content_stream, *m_page_handle);
	f->Show();
}


