#ifndef MYDOCUMENT_HPP_INCLUDED
#define MYDOCUMENT_HPP_INCLUDED
#include <libpdf/PDF.hpp>
#include <string>

class wxDC;
class WxTabDocument
{
	private:
		PDF::File file;
		PDF::Document * doc;
		PDF::Page * page;
		int m_rotation;
		double m_scale;
		int m_cur_page_num;
	public:
		WxTabDocument():doc(NULL),page(NULL),m_rotation(0),m_scale(1.0),m_cur_page_num(0) {}
		bool LoadFile(std::string fname);
		bool LoadPage(int num);
		void Draw(PDF::Media * mf);
		void Draw(wxDC * dc);
		int GetPagesNum() const { return doc?doc->get_pages_count():0; }
		int GetPageNum() const { return m_cur_page_num; }
		PDF::Page * GetPageObject() { return page; }
		void Refresh() { }
		void Rotate(int r) { m_rotation = r; Refresh(); }
		void Scale(unsigned int pr) { m_scale = pr/100.0; }
};

extern WxTabDocument * theDocument;


#endif /* MYDOCUMENT_HPP_INCLUDED */

