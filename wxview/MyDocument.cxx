#include "MyDocument.hpp"
#include <iostream>

MyDocument * theDocument = NULL;

bool MyDocument::LoadFile(std::string fname)
{
	if(doc) {
		delete page;
		delete doc;
		file.close();
	}

	file.debug(0);
  file.open(fname);
  if(! file.load()) {
		file.close();
		return false;
	}

	std::clog << "+ File loaded" << std::endl;

//  f.dump();

	try {
    doc = new PDF::Document(file, 5/*debug level*/);
		std::clog << "+ Document loaded" << std::endl;
//    PDF::Object::m_debug=true;
//    doc.dump();
		return true;
	}
  catch(std::string s) {
    std::cerr << "!Exception: " << s << std::endl;
  }
  catch(PDF::DocumentStructureException e) {
    std::cerr << "DocumentStructureException:\n  " << e.what() << std::endl;
  }
  catch(PDF::FormatException e) {
    std::cerr << "Format excertion:\n  " << e.what() << std::endl;
  }
  catch(std::exception e) {
    std::cerr << "Unknown exception:\n  " << e.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Unknown exception!" << std::endl;
  }
	/* reached only after exception */
	file.close();
	return false;
}

bool MyDocument::LoadPage(int num)
{
	if(page) delete page;
	try {
		page = new PDF::Page();
		page->debug(5);
		std::clog << "Loading " << num << "th page" << std::endl;
		page->load(doc->get_page_node(num-1));
		std::clog << page->dump() << std::endl;
		m_pagenum = num;
		return true;
	}
  catch(std::string s) {
    std::cerr << "!Exception: " << s << std::endl;
  }
  catch(PDF::DocumentStructureException e) {
    std::cerr << "DocumentStructureException:\n  " << e.what() << std::endl;
  }
  catch(PDF::FormatException e) {
    std::cerr << "Format excertion:\n  " << e.what() << std::endl;
  }
  catch(std::exception e) {
    std::cerr << "Unknown exception:\n  " << e.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Unknown exception!" << std::endl;
  }
	/* reached only after exception */
	return false;
}

void MyDocument::Draw(PDF::Media * mf)
{
	page->draw(mf);
}


