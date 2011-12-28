
//#include <fstream>
#include <string>
#include <iostream>
//#include <map>
#include <vector>
//#include <sstream>
#include "Document.hpp"
#include "Exceptions.hpp"
#include "OH.hpp"

namespace PDF {

//Document::Document(ObjectsCache * c, ObjId root_element, int debug):cache(c),m_debug(debug)
//Document::Document(File & f, int debug):m_debug(debug),cache(f),doc_root(cache.get_object(f.get_root()))
	Document::Document(File & f, int debug):m_debug(debug),cache(f)
	{
		if(m_debug>1) std::clog << "Constructing Document" << std::endl;
		if(f.can_load()) {
			doc_root=cache.get_object(f.get_root());

			// check type
			if(m_debug>1) std::clog << " Checking type..." << std::endl;
			OH type_h=doc_root.find("Type");
			if(!type_h) throw DocumentStructureException("No root elementi type defined");
			Name * type;
			type_h.put(type);
			if(!type || type->value()!="Catalog")
				throw DocumentStructureException("Invalid type of root element");

			// get pages reference
			if(m_debug>1) std::clog << " Finding pages reference..." << std::endl;
			OH pages_h=doc_root.find("Pages");
			if(!pages_h) throw DocumentStructureException("No pages");

			if(m_debug>1) std::clog << " Reading pages index..." << std::endl;
			// read all pages (just indices, contents loaded upon request)
			parse_pages_tree(pages_h);
		}
		else
		{
			next_free_object_id.num = next_free_object_id.gen = 0;
			initialise_new();
		}
	}

Document::~Document()
{
}

/// Outputs document contents for debug purposes
void Document::dump()
{
  std::cout << "Document dump:" << std::endl;
//  std::cout << dump_objects_cache();
  std::cout << dump_pages_directory();
  std::cout << "  " << get_pages_count() << " pages" << std::endl;
//  std::cout << dump_objects_cache();
}

void Document::parse_pages_tree(OH pagenode_h)
{
	pagenode_h.expand(); // dereference indirect objects
	if(!dynamic_cast<Dictionary *>(pagenode_h.obj()))
		throw DocumentStructureException("Invalid Pages node format");

	// get node type
	OH type_h=pagenode_h.find("Type"); if(!type_h) throw DocumentStructureException("No type");
	Name * n;
	type_h.put(n, "pages node type is not a name but is a ");

	if(n->value() == "Pages")
	{
		OH kids_h=pagenode_h.find("Kids");
		if(!kids_h)
			throw DocumentStructureException("No kids");
		if(!dynamic_cast<Array *>(kids_h.obj()))
			throw DocumentStructureException("Page node Kids is not an array");

		// recurse into all kids
		for(unsigned int i=0; i<kids_h.size(); i++)
			parse_pages_tree(kids_h[i]);
	} else if(n->value() == "Page") {
//		std::clog << "Found page: " << pagenode_h.obj()->dump() << ")" << std::endl;
		all_pages.push_back(pagenode_h.id()); // insert reference of page node
	} else
		throw DocumentStructureException("Unknown pages tree node type");
}

OH Document::get_object(const ObjId & id)
{
//  return cache->get_object(id);
  return cache.get_object(id);
}

PDF::ObjId Document::new_object_id()
{
	ObjId r = next_free_object_id;
	++next_free_object_id.num;
	return r;
}

PDF::OH Document::new_indirect_object( Object * o )
{
	o->indirect = true;
	o->m_id = new_object_id();
	return cache.new_object(o).set_modified();
}

void Document::initialise_new()
{
	next_free_object_id.num = 1;

	// Create main catalog
	PDF::Dictionary * root = new PDF::Dictionary();
	root->set("Type", new PDF::Name("Catalog"));
	doc_root = new_indirect_object(root);
}

PDF::OH Document::build_pages_tree()
{
	Dictionary * d = new Dictionary;
	d->set("Type", new Name("Pages"));
	OH r = new_indirect_object(d);
	PDF::Array * pages_array = new PDF::Array;
	for(std::vector<ObjId>::iterator it = all_pages.begin(); it != all_pages.end(); ++it) {
		pages_array->push(new PDF::ObjRef(*it));
		Dictionary * d;
		get_object(*it).put(d, "Page object is not a Dictionary?");
		d->set("Parent", new ObjRef(r->m_id));
	}
	d->set("Count", new PDF::Integer(pages_array->size()));
	d->set("Kids", pages_array);
	return r;
}

PDF::OH Document::add_page()
{
	Dictionary * d = new Dictionary;
	d->set("Type", new PDF::Name("Page"));
	//d->set("Parent", new PDF::ObjRef(pages_id));
	d->set("Rotate", new PDF::Integer(0));
	//d->set("Content", new PDF::ObjRef(page_content_id));
	OH h = new_indirect_object(d);
	all_pages.push_back(h->m_id);
	return h;
}

void Document::save()
{
	PDF::Dictionary * root;
	doc_root.put(root, "Document root node must be a Dictionaty");
	OH pages = build_pages_tree();
	root->set("Pages", new ObjRef(pages->m_id));

	cache.save_modified();
	cache.file().set_root(doc_root->m_id);
}


} // namespace PDF


