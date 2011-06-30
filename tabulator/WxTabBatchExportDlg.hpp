#ifndef WXTABBATCHEXPORTDLG_HPP_INCLUDED
#define WXTABBATCHEXPORTDLG_HPP_INCLUDED

#include <wx/wx.h>

class WxTabDocument;
class PageNumCtrl;
class WxTabBatchExportDialog:public wxDialog
{
	enum { ID_StartExport = 10001, };
	DECLARE_EVENT_TABLE();
public:
	WxTabBatchExportDialog(wxWindow * parent, WxTabDocument * document);
	~WxTabBatchExportDialog();
	void OnStartExport(wxCommandEvent &event);
	void SetCurPage(int num);
private:
	WxTabDocument * document;
	PageNumCtrl * page1, * page2;
	wxStaticText * page_label1, * page_label2;
	wxRadioBox * exporterselect;
	wxTextCtrl * expopts;
	wxStaticText * expopts_label;
};

#endif /* WXTABBATCHEXPORTDLG_HPP_INCLUDED */
