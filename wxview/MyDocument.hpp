#ifndef MYDOCUMENT_HPP_INCLUDED
#define MYDOCUMENT_HPP_INCLUDED
#include "../PDF.hpp"
#include <string>

class MyDocument
{
	private:
		PDF::File file;
		PDF::Document * doc;
		PDF::Page * page;
		unsigned int m_pagenum;
	public:
		MyDocument():doc(NULL),page(NULL),m_pagenum(0) {}
		bool LoadFile(std::string fname);
		bool LoadPage(int num);
		void Draw(PDF::Media * mf);
		int GetPagesNum() const { return doc?doc->get_pages_count():0; }
		unsigned int GetPageNum() const { return m_pagenum; }
		PDF::Page * GetPageObject() { return page; }
};

extern MyDocument * theDocument;


#endif /* MYDOCUMENT_HPP_INCLUDED */

