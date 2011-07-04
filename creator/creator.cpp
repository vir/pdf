// creator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "libpdf/File.hpp"
#include "libpdf/Object.hpp"
#include "libpdf/Document.hpp"
#include "libpdf/OH.hpp"
#include <iostream>


int main(int argc, char * argv[])
{
	try {
		std::clog << "Constructing File" << std::endl;
		PDF::File pf(1.3, "test.pdf");
		pf.debug(3);

		long id_count = 1;
		PDF::ObjId oi = { 0, 0 };

		PDF::Array * pages_array = new PDF::Array;
		// XXX Add pages into array

		PDF::Dictionary * pages = new PDF::Dictionary;
		pages->set("Type", new PDF::Name("Pages"));
		pages->set("Count", new PDF::Integer(pages_array->size()));
		pages->set("Kids", pages_array);
		oi.num = id_count++;
		pages->m_id = oi;
		pages->indirect = true;
		pf.save_object(pages, oi);

		PDF::Object * cat = new PDF::Name("Catalog");
		PDF::Dictionary * root = new PDF::Dictionary();
		root->set("Type", cat);
		root->set("Pages", new PDF::ObjRef(oi));
		oi.num = id_count++;
		root->m_id = oi;
		root->indirect = true;
		pf.save_object(root, oi);
		pf.set_root(oi);

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

