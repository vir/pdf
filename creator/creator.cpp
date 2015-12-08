// creator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "libpdf/File.hpp"
#include "libpdf/Object.hpp"
#include "libpdf/Document.hpp"
#include "libpdf/OH.hpp"
#include <iostream>

void read_file(const char * filename, std::vector<char> & vec)
{
	using namespace std;
	ifstream ifs(filename, ios::in | ios::binary | ios::ate);

	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	vec.resize(fileSize);
	ifs.read(&vec[0], fileSize);
}

int main(int argc, char * argv[])
{
	try {
		std::clog << "Constructing File" << std::endl;
		PDF::File pf(1.3);
		pf.open("test.pdf", PDF::File::MODE_CREATE);
		pf.debug(3);

#if 0
		PDF::ObjId root_dict_id = { 4, 0 };
		PDF::ObjId pages_array_id = { 3, 0 };
		PDF::ObjId page1_dict_id = { 2, 0 };
		PDF::ObjId page1_content_id = { 1, 0 };

		// XXX create page content stream

		// Create page dictionary
		PDF::Dictionary * page1_dict = new PDF::Dictionary;
		page1_dict->set("Type", new PDF::Name("Page"));
		page1_dict->set("Parent", new PDF::ObjRef(pages_array_id));
		page1_dict->set("Rotate", new PDF::Integer(0));
		page1_dict->set("Content", new PDF::ObjRef(page1_content_id));
		page1_dict->m_id = page1_dict_id;
		page1_dict->indirect = true;
		pf.save_object(page1_dict, page1_dict_id);

		// Create pages array
		PDF::Dictionary * pages = new PDF::Dictionary;
		pages->set("Type", new PDF::Name("Pages"));
		PDF::Array * pages_array = new PDF::Array;
		pages_array->push(new PDF::ObjRef(page1_dict_id));
		pages->set("Count", new PDF::Integer(pages_array->size()));
		pages->set("Kids", pages_array);
		pages->m_id = pages_array_id;
		pages->indirect = true;
		pf.save_object(pages, pages_array_id);

		// Create main catalog
		PDF::Object * cat = new PDF::Name("Catalog");
		PDF::Dictionary * root = new PDF::Dictionary();
		root->set("Type", cat);
		root->set("Pages", new PDF::ObjRef(pages_array_id));
		root->m_id = root_dict_id;
		root->indirect = true;
		pf.save_object(root, root_dict_id);
		pf.set_root(root_dict_id);
#else
		PDF::Document doc(pf);
		PDF::OH p1 = doc.add_page();

		PDF::Stream * s1 = new PDF::Stream();
#if 0
		std::string body("0 G\n1 J 1 j 0.72 w 10 M []0 d\n1 i\n272.64 649.52 m\n272.64 727.16 l\nS\n");
		std::vector<char> buf(body.begin(), body.end());
#else
		std::vector<char> buf;
		read_file("page.txt", buf);
#endif
		s1->put_data(buf);
		PDF::OH h1 = doc.new_indirect_object(s1);
		PDF::Dictionary * pd;
		p1.put(pd);
		pd->set("Contents", new PDF::ObjRef(h1->m_id));

		PDF::Array * mediabox = new PDF::Array;
		mediabox->push(new PDF::Integer(0));
		mediabox->push(new PDF::Integer(0));
		mediabox->push(new PDF::Integer(595));
		mediabox->push(new PDF::Integer(842));
		pd->set("MediaBox", mediabox);

		// Create font (use standard Helvetica font)
		PDF::Dictionary * font1 = new PDF::Dictionary;
		font1->set("Type", new PDF::Name("Font"));
		font1->set("Subtype", new PDF::Name("Type1"));
		font1->set("BaseFont", new PDF::Name("Helvetica"));
		font1->set("Encoding", new PDF::Name("WinAnsiEncoding"));
		PDF::OH fh = doc.new_indirect_object(font1);

		// Create page descriptor
		PDF::Dictionary * resdict = new PDF::Dictionary;
		PDF::Array * procset = new PDF::Array;
		procset->push(new PDF::Name("PDF"));
		procset->push(new PDF::Name("Text"));
		resdict->set("ProcSet", procset);
		PDF::Dictionary * fonts = new PDF::Dictionary;
		fonts->set("F1", new PDF::ObjRef(fh->m_id));
		resdict->set("Font", fonts);
		pd->set("Resources", resdict);

		doc.save();
#endif

		pf.close();
	}
	catch(std::exception & e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	catch(...) {
		std::cerr << "Unknown Exception!" << std::endl;
	}
	return 0;
}

