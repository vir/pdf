#ifndef PAGENUMCTRL_HPP_INCLUDED
#define PAGENUMCTRL_HPP_INCLUDED

#include <wx/wx.h>

class PageNumCtrl:public wxTextCtrl
{
	private:
		long num;
		long min, max;
	public:
		PageNumCtrl(wxWindow* parent, wxWindowID id, const wxString& value = _T(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTE_CENTRE|wxTE_PROCESS_ENTER, const wxString& name = wxTextCtrlNameStr):wxTextCtrl(parent, id, value, pos, size, style, wxDefaultValidator, name),num(0),min(0),max(10000)
		{
		}
		void OnChar( wxKeyEvent & event )
		{
#if 1
			wxString backup = wxTextCtrl::GetValue();
			long tmp;
			if( event.GetKeyCode() < 32 || event.GetKeyCode() == 127 || event.GetKeyCode() > 256) event.Skip();
			if( event.GetKeyCode() > 255 || !isdigit( event.GetKeyCode() ) ) return;
//			EmulateKeyPress(event); // causes infinite recursion on win32
			event.Skip();
			wxString newval = wxTextCtrl::GetValue();
			if( !newval.ToLong(&tmp) || tmp<min || tmp>max ) {
				wxTextCtrl::SetValue(backup);
				wxBell();
			} else {
				num = tmp;
			}
#else
			event.Skip();
#endif
		}
		void OnSetFocus( wxFocusEvent & event )
		{
			wxTextCtrl::SetSelection(-1, -1);
		}
		void SetRange(int r1, int r2)
		{
			min = r1;
			max = r2;
		}
		void SetValue(int i)
		{
			num = i;
			wxTextCtrl::SetValue(wxString::Format(_T("%d"), i));
		}
		int GetValue()
		{
			long tmp;
			wxString newval = wxTextCtrl::GetValue();
			if( newval.ToLong(&tmp) || tmp<min || tmp>max ) {
				num = tmp;
			}
			return num;
		}
    DECLARE_EVENT_TABLE()
};

#endif /* PAGENUMCTRL_HPP_INCLUDED */
