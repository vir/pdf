#include "wx/wx.h"
#include "PdfExplorerView.hpp"
#include "pdfdig.hpp"

IMPLEMENT_DYNAMIC_CLASS(PdfExplorerView, wxView)

BEGIN_EVENT_TABLE(PdfExplorerView, wxView)
//	EVT_MENU(DOODLE_CUT, PdfExplorerView::OnCut)
END_EVENT_TABLE()


PdfExplorerView::PdfExplorerView()
	:m_splitter(NULL),m_tree(NULL),m_right(NULL),frame(NULL)
{
}

PdfExplorerView::~PdfExplorerView()
{
	delete m_right;
	delete m_tree;
	delete m_splitter;
}


#if 0

#endif

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool PdfExplorerView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
std::cerr << "PdfExplorerView::OnCreate" << std::endl;
	// Associate the appropriate frame with this view.
	frame = static_cast<MyApp*>(wxTheApp)->GetMainFrame();
	SetFrame(frame);

	m_splitter = new wxSplitterWindow(frame, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ );
	m_splitter->SetSashGravity(1.0);

	m_tree = new MyTree(this, m_splitter);
	m_right = new wxTextCtrl(m_splitter, wxID_ANY, _T("second text"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP);
	m_tree->m_details = m_right;

	m_splitter->SplitVertically(m_tree, m_right, 300);

	// Make sure the document manager knows that this is the
	// current view.
	Activate(true);

#if 0
	// Initialize the edit menu Undo and Redo items
	doc->GetCommandProcessor()->SetEditMenu(((MyFrame *)frame)->editMenu);
	doc->GetCommandProcessor()->Initialize();
#endif
	return true;
}

// Used for default print/preview as well as drawing on the screen.
void PdfExplorerView::OnDraw(wxDC *dc)
{
std::cerr << "PdfExplorerView::OnDraw" << std::endl;
	dc->SetFont(*wxNORMAL_FONT);
	dc->SetPen(*wxBLACK_PEN);

}

void PdfExplorerView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint))
{
std::cerr << "PdfExplorerView::OnUpdate" << std::endl;
	m_tree->Update();
//	m_splitter->Refresh();
#if 0
    if (canvas)
        canvas->Refresh();
    
/* Is the following necessary?
#ifdef __WXMSW__
    if (canvas)
        canvas->Refresh();
#else
    if (canvas)
    {
        wxClientDC dc(canvas);
        dc.Clear();
        OnDraw(& dc);
    }
#endif
*/
#endif
}

// Clean up windows used for displaying the view.
bool PdfExplorerView::OnClose(bool deleteWindow)
{
std::cerr << "PdfExplorerView::OnClose" << std::endl;
	if (!GetDocument()->Close())
		return false;

	wxString s(wxTheApp->GetAppName());
	if (frame)
		frame->SetTitle(s);

	SetFrame((wxFrame *) NULL);

	Activate(false);

	return true;
}

#if 0
void PdfExplorerView::OnCut(wxCommandEvent& WXUNUSED(event) )
{
	DrawingDocument *doc = (DrawingDocument *)GetDocument();
	doc->GetCommandProcessor()->Submit(new DrawingCommand(_T("Cut Last Segment"), DOODLE_CUT, doc, (DoodleSegment *) NULL));
}
#endif


