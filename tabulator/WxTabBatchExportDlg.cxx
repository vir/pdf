#include "WxTabBatchExportDlg.hpp"
#include "PageNumCtrl.hpp"
#include "Tabulator_Exporter.hpp"
#include "WxTabDocument.hpp"
#include <wx/progdlg.h>

WxTabBatchExportDialog::WxTabBatchExportDialog( wxWindow * parent, WxTabDocument * document )
: wxDialog(parent, wxID_ANY, _("Batch process"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
, document(document)
{
	wxBoxSizer * s = new wxBoxSizer( wxVERTICAL /* wxHORIZONTAL */ );

	wxBoxSizer * s_pages = new wxBoxSizer( wxHORIZONTAL );
	page_label1 = new wxStaticText(this, wxID_ANY, _T("Pages from "));
	s_pages->Add(page_label1);
	page1 = new PageNumCtrl(this, wxID_ANY);
	page1->SetRange(1, document->GetPagesNum());
	s_pages->Add(page1);
	page_label2 = new wxStaticText(this, wxID_ANY, _T(" to "));
	s_pages->Add(page_label2);
	page2 = new PageNumCtrl(this, wxID_ANY);
	page2->SetRange(1, document->GetPagesNum());
	s_pages->Add(page2);
	s->Add(s_pages);

#ifdef _WIN32
	wxString exporterChoices[3];
#else
	wxString exporterChoices[2];
#endif
	exporterChoices[0] = _("&CSV");
	exporterChoices[1] = _("&XML");
#ifdef _WIN32
	exporterChoices[2] = _("&Excel");
#endif
	exporterselect = new wxRadioBox(this, wxID_ANY, _("Export to:"), wxDefaultPosition, wxDefaultSize, sizeof(exporterChoices)/sizeof(exporterChoices[0]), exporterChoices);
	exporterselect->SetSelection(0);
	s->Add(exporterselect/*, 0, wxGROW|wxALL, 5*/);

	wxBoxSizer * s_eopts = new wxBoxSizer( wxHORIZONTAL );
	expopts_label = new wxStaticText(this, wxID_ANY, _T("Export options: "));
	s_eopts->Add(expopts_label);
	expopts = new wxTextCtrl(this, wxID_ANY);
	s_eopts->Add(expopts);
	s->Add(s_eopts);

	wxBoxSizer * s_buttons = new wxBoxSizer( wxHORIZONTAL );
	s_buttons->Add(new wxButton( this, wxID_OK, _("Close") ));
	s_buttons->Add(new wxButton( this, ID_StartExport, _T("Start export") ));
	s->Add(s_buttons);

	SetAutoLayout( TRUE );
	SetSizer( s );
	s->Fit( this );
	s->SetSizeHints( this );
}

WxTabBatchExportDialog::~WxTabBatchExportDialog()
{
	delete page1;
	delete page2;
	delete page_label1;
	delete page_label2;
	delete exporterselect;
	delete expopts;
	delete expopts_label;
}

BEGIN_EVENT_TABLE(WxTabBatchExportDialog, wxDialog)
EVT_BUTTON(ID_StartExport, WxTabBatchExportDialog::OnStartExport)
END_EVENT_TABLE()

void WxTabBatchExportDialog::OnStartExport( wxCommandEvent &event )
{
	Tabulator::Table::Exporter * exporter = NULL;
	switch(exporterselect->GetSelection()) {
		case 0:
			exporter = new ExporterCSV(std::cout);
			break;
		case 1:
			exporter = new ExporterHTML(std::cout);
			break;
#ifdef _WIN32
		case 2:
			exporter = new ExporterExcel();
			if(!static_cast<ExporterExcel*>(exporter)->get_active()) {
				static_cast<ExporterExcel*>(exporter)->start_new();
				static_cast<ExporterExcel*>(exporter)->add_workbook();
			}
			static_cast<ExporterExcel*>(exporter)->set_visible(true);
			break;
#endif
		default:
			return;
	}
	wxString expparams = expopts->GetValue();
	if(expparams.Len()) {
		if(!exporter->set_params(static_cast<const char *>(expparams.utf8_str()))) {
			std::cerr << "Error setting exporter params" << std::endl;
			return;
		}
	}

	int page_first = page1->GetValue();
	int page_last = page2->GetValue();

	wxProgressDialog progr(_T("Exporting"), _T("Exporting document"), page_last - page_first + 1, this, wxPD_APP_MODAL|wxPD_SMOOTH|wxPD_CAN_ABORT|wxPD_ELAPSED_TIME|wxPD_ESTIMATED_TIME|wxPD_REMAINING_TIME);
	for(int pagenum = page_first; pagenum <= page_last; pagenum++) // all needed
	{
		if(! progr.Update(pagenum - page_first, wxString::Format(_T("Exporting page %d"), pagenum)))
			break;
		exporter->page_begin(static_cast<const char *>(document->GetName().utf8_str()), pagenum);
		document->ExportPage(pagenum, exporter);
		exporter->page_end();
	}
	progr.Update(page_last - page_first + 1, _T("Finished"));
	delete exporter;
}

void WxTabBatchExportDialog::SetCurPage( int num )
{
	page1->SetValue(num);
	page2->SetValue(num);
}



