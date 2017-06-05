#ifndef WXTABBATCHEXPORTDLG_HPP_INCLUDED
#define WXTABBATCHEXPORTDLG_HPP_INCLUDED

#include <wx/wx.h>

class WxTabDocument;
class PageNumCtrl;
class WxTabBatchExportDialog:public wxDialog
{
	enum { ID_StartExport = 10001, ID_ExportSwitch };
	DECLARE_EVENT_TABLE();
public:
	WxTabBatchExportDialog(wxWindow * parent, WxTabDocument * document);
	~WxTabBatchExportDialog();
	void SetPages(int first, int last, int in_row = 1);
	void OnStartExport(wxCommandEvent &event);
	void OnExportSwitchChanged(wxCommandEvent &event);
	void SetCurPage(int num);
protected:
	wxString MakeExportedName();
private:
	WxTabDocument * document;
	wxBoxSizer * top_sizer;
	wxBoxSizer * s_eopts_default;
	PageNumCtrl * page1, * page2;
	wxRadioBox * exporterselect;
	wxTextCtrl * expopts;
#ifdef _WIN32
	wxBoxSizer *s_eopts_excel;
	wxCheckBox *excel_background, *excel_save, *excel_page_numbers;
	wxSpinCtrl *excel_pages_in_row;
#endif
};

#endif /* WXTABBATCHEXPORTDLG_HPP_INCLUDED */
