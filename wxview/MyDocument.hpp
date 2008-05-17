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
	public:
		MyDocument():doc(NULL),page(NULL) {}
		bool LoadFile(std::string fname);
		bool LoadPage(int num);
		void Draw(PDF::Media * mf);
		int GetPagesNum() const { return doc?doc->get_pages_count():0; }
		PDF::Page * GetPageObject() { return page; }
};

extern MyDocument * theDocument;


#endif /* MYDOCUMENT_HPP_INCLUDED */

