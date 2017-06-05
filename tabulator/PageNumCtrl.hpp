#ifndef PAGENUMCTRL_HPP_INCLUDED
#define PAGENUMCTRL_HPP_INCLUDED

#include <wx/wx.h>

class PageNumCtrl:public wxTextCtrl
{
	private:
		long num;
		long min, max;
	public:
		PageNumCtrl(wxWindow* parent, wxWindowID id, const wxString& value = _T(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTE_CENTRE | wxTE_PROCESS_ENTER, const wxString& name = wxTextCtrlNameStr);
		void OnChar(wxKeyEvent & event);
		void OnSetFocus(wxFocusEvent & event);
		void SetRange(int r1, int r2);
		void SetValue(int i);
		int GetValue();
    DECLARE_EVENT_TABLE()
};

#endif /* PAGENUMCTRL_HPP_INCLUDED */
