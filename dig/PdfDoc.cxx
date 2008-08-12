#include "PdfDoc.hpp"

IMPLEMENT_DYNAMIC_CLASS(PdfDoc, wxDocument)

PdfDoc::PdfDoc()
	:cache(file)
{
std::cerr << "Created document" << std::endl;
}

bool PdfDoc::OnOpenDocument(const wxString& filename)
{
std::cerr << "OnOpenDocument" << std::endl;
	cache.flush();
	if(! file.open(std::string(filename.mb_str())))
		return false;
std::cerr << "Opened file, loading" << std::endl;
	if(! file.load())
		return false;
std::cerr << "Loaded file" << std::endl;
file.dump();

	SetFilename(filename, true);
	Modify(false);
	UpdateAllViews();
	return true;
}

bool PdfDoc::OnSaveDocument(const wxString& filename)
{
	return false;
}

bool PdfDoc::OnCloseDocument()
{
	return file.close();
}

PDF::OH PdfDoc::get_object(const PDF::ObjId & id)
{
	return cache.get_object(id);
}

#if 0
bool PdfDoc::open(std::string fname)
{
	return true;
}

void PdfDoc::close()
{
}
#endif



