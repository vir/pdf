#include "wx/wx.h"
#include "PdfExplorerView.hpp"
#include "pdfdig.hpp"

IMPLEMENT_DYNAMIC_CLASS(PdfExplorerView, wxView)

BEGIN_EVENT_TABLE(PdfExplorerView, wxView)
//	EVT_MENU(DOODLE_CUT, PdfExplorerView::OnCut)
END_EVENT_TABLE()


PdfExplorerView::PdfExplorerView()
	:m_splitter(NULL),m_tree(NULL),m_right(NULL), m_frame(NULL)
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

	m_tree = new MyTree(this, m_splitter);
	m_right = new wxTextCtrl(m_splitter, wxID_ANY, _T("second text"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP);
	m_tree->m_details = m_right;

	subframe->Maximize();
	m_splitter->SplitVertically(m_tree, m_right, 300);
}

PdfExplorerView::~PdfExplorerView()
{
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



