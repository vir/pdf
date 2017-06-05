#include "WxTabBatchExportDlg.hpp"
#include "PageNumCtrl.hpp"
#include "Tabulator_Exporter.hpp"
#include "WxTabDocument.hpp"
#include <wx/progdlg.h>
#include <wx/spinctrl.h>

WxTabBatchExportDialog::WxTabBatchExportDialog( wxWindow * parent, WxTabDocument * document )
: wxDialog(parent, wxID_ANY, _("Batch process"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
, document(document)
{
	top_sizer = new wxBoxSizer( wxVERTICAL /* wxHORIZONTAL */ );

	wxBoxSizer * row = new wxBoxSizer( wxHORIZONTAL );
	row->Add(new wxStaticText(this, wxID_ANY, _T("Pages from ")));
	page1 = new PageNumCtrl(this, wxID_ANY);
	page1->SetRange(1, document->GetPagesNum());
	row->Add(page1);
	row->Add(new wxStaticText(this, wxID_ANY, _T(" to ")));
	page2 = new PageNumCtrl(this, wxID_ANY);
	page2->SetRange(1, document->GetPagesNum());
	row->Add(page2);
	top_sizer->Add(row);

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
	exporterselect = new wxRadioBox(this, ID_ExportSwitch, _("Export to:"), wxDefaultPosition, wxDefaultSize, sizeof(exporterChoices)/sizeof(exporterChoices[0]), exporterChoices);
	exporterselect->SetSelection(0);
	top_sizer->Add(exporterselect/*, 0, wxGROW|wxALL, 5*/);

	s_eopts_default = new wxBoxSizer( wxHORIZONTAL );
	s_eopts_default->Add(new wxStaticText(this, wxID_ANY, _T("Export options: ")));
	expopts = new wxTextCtrl(this, wxID_ANY);
	s_eopts_default->Add(expopts);
	top_sizer->Add(s_eopts_default);

#ifdef _WIN32
	{
		s_eopts_excel = new wxBoxSizer( wxVERTICAL );
		s_eopts_excel->Add(new wxStaticText(this, wxID_ANY, _T("Excel options: ")));
		excel_background = new wxCheckBox(this, wxID_ANY, wxT("Background operation"));
		s_eopts_excel->Add(excel_background);
		excel_page_numbers = new wxCheckBox(this, wxID_ANY, wxT("Insert page numbers"));
		s_eopts_excel->Add(excel_page_numbers);
		excel_save = new wxCheckBox(this, wxID_ANY, wxString(wxT("Save file ")) + MakeExportedName());
		s_eopts_excel->Add(excel_save);

		row = new wxBoxSizer( wxHORIZONTAL );
		row->Add(new wxStaticText(this, wxID_ANY, _T("Pages in row: ")));
		excel_pages_in_row = new wxSpinCtrl(this, wxID_ANY);
		row->Add(excel_pages_in_row);
		s_eopts_excel->Add(row);

		s_eopts_excel->Show(false);
		top_sizer->Add(s_eopts_excel);
	}
#endif

	row = new wxBoxSizer( wxHORIZONTAL );
	row->Add(new wxButton( this, wxID_OK, _("Close") ));
	row->Add(new wxButton( this, ID_StartExport, _T("Start export") ));
	top_sizer->Add(row);

	SetAutoLayout( TRUE );
	SetSizer( top_sizer );
	top_sizer->Fit( this );
	top_sizer->SetSizeHints( this );
}

WxTabBatchExportDialog::~WxTabBatchExportDialog()
{
}

void WxTabBatchExportDialog::SetPages(int first, int last, int in_row /* = 1 */)
{
	page1->SetValue(first);
	page2->SetValue(last);
#ifdef _WIN32
	excel_pages_in_row->SetValue(in_row);
#endif

}

BEGIN_EVENT_TABLE(WxTabBatchExportDialog, wxDialog)
EVT_BUTTON(ID_StartExport, WxTabBatchExportDialog::OnStartExport)
EVT_RADIOBOX(ID_ExportSwitch, WxTabBatchExportDialog::OnExportSwitchChanged)
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
			{
				ExporterExcel* e = new ExporterExcel();
				if(!e->get_active()) {
					e->start_new();
					e->add_workbook();

					int pgs = excel_pages_in_row->GetValue();
					if(pgs)
						e->set_sheets_number(pgs);
					e->set_insert_page_numbers(excel_page_numbers->GetValue());
				}
				e->set_visible(! excel_background->GetValue());
				exporter = e;
			}
			break;
#endif
		default:
			return;
	}
#if 0
	wxString expparams = expopts->GetValue();
	if(expparams.Len()) {
		if(!exporter->set_params(static_cast<const char *>(expparams.utf8_str()))) {
			std::cerr << "Error setting exporter params" << std::endl;
			return;
		}
	}
#endif
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
#ifdef _WIN32
	if(exporterselect->GetSelection() == 2)
	{
		ExporterExcel* e = static_cast<ExporterExcel*>(exporter);
		e->set_visible(true);
		if(excel_save->GetValue())
			e->save_as(MakeExportedName().wc_str());
	}
#endif
	progr.Update(page_last - page_first + 1, _T("Finished"));
	delete exporter;
}

void WxTabBatchExportDialog::OnExportSwitchChanged(wxCommandEvent &event)
{
#ifdef _WIN32
	int someint = event.GetInt();
	if(someint == 2)
	{
		s_eopts_excel->Show(true);
		s_eopts_default->Show(false);
	}
	else
	{
		s_eopts_excel->Show(false);
		s_eopts_default->Show(true);
	}
	top_sizer->Layout();
	top_sizer->Fit( this );
#endif
}

void WxTabBatchExportDialog::SetCurPage( int num )
{
	page1->SetValue(num);
	page2->SetValue(num);
}

wxString WxTabBatchExportDialog::MakeExportedName()
{
	wxString n = theDocument->GetName();
	if(n.Lower().EndsWith(wxT(".pdf")))
		n.Truncate(n.Length() - 4);
	return n + wxT(".xlsx");
}



