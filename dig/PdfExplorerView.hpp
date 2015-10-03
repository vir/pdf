#ifndef PDFEXPLORERVIEW_HPP_INCLUDED
#define PDFEXPLORERVIEW_HPP_INCLUDED

#include <wx/wx.h>
#include "wx/splitter.h"
#include "wx/docview.h"
#include "MyTree.hpp"
#include "MyFrame.hpp"

class PdfExplorerView:public wxView, public MyTreeEventsHandler
{
	DECLARE_DYNAMIC_CLASS(PdfExplorerView)
	private:
		wxSplitterWindow * m_splitter;
		MyTree * m_tree;
		wxTextCtrl * m_right;
		wxFrame * m_frame;
		MyFrame * m_mainframe;
		PDF::OH * m_stream_handle, * m_stream_parent_handle;
	public: // MyTreeEventHandler
		virtual void SelectedNothing();
		virtual void SelectedObject(PDF::OH h, PDF::OH parent);
	public:
		void ViewStreamData();
		void SaveStreamData(std::ostream& ostr);
	public:
		PdfExplorerView();
		~PdfExplorerView();
		bool OnCreate(wxDocument *doc, long flags);
		void OnDraw(wxDC *dc);
		void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
		bool OnClose(bool deleteWindow = true);
		DECLARE_EVENT_TABLE()
};


#endif /* PDFEXPLORERVIEW_HPP_INCLUDED */

