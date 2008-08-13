#ifndef PDFDIG_HPP_INCLUDED
#define PDFDIG_HPP_INCLUDED

#include <wx/wx.h>
#include "wx/docview.h"
#include "MyFrame.hpp"

class MyApp : public wxApp
{
	private:
//		wxString fname;
	protected:
		wxDocManager* m_docManager;
		MyFrame * frame;
	public:
		MyFrame * GetMainFrame() const { return frame; }
		MyApp():m_docManager(NULL),frame(NULL) {}
		virtual bool OnInit();
		virtual int OnExit();
		virtual void OnInitCmdLine(wxCmdLineParser& parser);
		virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
};



#endif /* PDFDIG_HPP_INCLUDED */

