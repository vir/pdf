#ifndef MYSTREAMVIEWER_H_INCLUDED
#define MYSTREAMVIEWER_H_INCLUDED

#include "wx/wx.h"
#include "wx/docview.h"
#include "wx/splitter.h"
#include <PDF.hpp>

class wxSpinEvent;
class wxSpinCtrl;
class MyCanvas;

class MyTextWidget: public wxTextCtrl
{
public:
	MyTextWidget(wxWindow * parent)
		: wxTextCtrl(parent, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP)
	{
	}
	// Some ideas stolen from http://wxwidgets.info/find_and_replace_for_wxwidgets_ru/
	bool Find(wxString substring, bool backwards = false)
	{
		return false;
	}
};

class MyStreamViewer: public wxFrame
{
public:
	MyStreamViewer(wxView* view, PDF::OH& h, PDF::OH& parenth);
	void OnSave(wxCommandEvent& event);
	void OnDebug(wxCommandEvent& WXUNUSED(event));
	void OnLog(wxCommandEvent& event);
	void OnBreak(wxCommandEvent& event);
	void OnOpLimit(wxSpinEvent& WXUNUSED(event));
	void OnScale(wxCommandEvent& event);

	void UpdateLogWindow();

	void SelectOperator( unsigned int lim );

	unsigned int CountMissedCRChars( unsigned int lim );

private:
	void DumpObject();
	inline bool is_ok(char c)
	{
		switch(c) {
		case 0x0D:
		case 0x0A:
			break;
		default:
			if(c < 0x20 || c >= 0x7F)
				return false;
			break;
		}
		return true;
	}
	wxSplitterWindow * m_split_log;
	wxSplitterWindow * m_splitter;
	PDF::OH& m_oh;
	PDF::OH& m_parenth;
	PDF::Page * m_page;
	wxView* m_view;
	std::vector<char> m_buf;
	wxTextCtrlBase* m_text;
	wxToolBarBase* m_toolBar;
	wxSpinCtrl* m_oplimitspin;
	MyCanvas* m_canvas;
	wxTextCtrl* m_logwin;
	DECLARE_EVENT_TABLE()
	bool ParsePage();
};

#endif /* MYSTREAMVIEWER_H_INCLUDED */
