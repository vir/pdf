
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
	doc_root=cache.get_object(f.get_root());

  // get root node
//  doc_root=cache->get_object(root_element); // Only last version!
  
  // check type
  if(m_debug>1) std::clog << " Checking type..." << std::endl;
  OH type_h=doc_root.find("Type");
  if(!type_h) throw DocumentStructureException("No root elementi type defined");
  Name * type;
  type_h.put(type);
  if(!type || type->value()!="Catalog") throw DocumentStructureException("Invalid type of root element");

  // get pages reference
  if(m_debug>1) std::clog << " Finding pages reference..." << std::endl;
  OH pages_h=doc_root.find("Pages");
  if(!pages_h) throw DocumentStructureException("No pages");
  
  if(m_debug>1) std::clog << " Reading pages index..." << std::endl;
  // read all pages (just indices, contents loaded upon request)
  parse_pages_tree(pages_h);
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
//  std::clog << "@ Document::parse_pages_tree(...)" << std::endl;
//  std::clog << dump_objects_cache();
  pagenode_h.expand(); // dereference indirect objects
  pagenode_h.cast<Dictionary *>("Invalid Pages node format");

  // get node type
  OH type_h=pagenode_h.find("Type"); if(!type_h) throw DocumentStructureException("No type");
  Name * n;
  type_h.put(n, "pages node type is not a name but is a ");

  if(n->value() == "Pages")
  {
    OH kids_h=pagenode_h.find("Kids"); if(!kids_h) throw DocumentStructureException("No kids");
    kids_h.cast<Array *>("Kids is not an array");

    // recurse into all kids
    for(unsigned int i=0; i<kids_h.size(); i++)
    {
      parse_pages_tree(kids_h[i]);
    }
  }
  else if(n->value() == "Page")
  {
//    std::clog << "Found page: " << pagenode_h.obj()->dump() << ")" << std::endl;
    all_pages.push_back(pagenode_h.id()); // insert reference of page node

  }
  else throw DocumentStructureException("Unknown pages tree node type");
}

OH Document::get_object(const ObjId & id)
{
//  return cache->get_object(id);
  return cache.get_object(id);
}
 
} // namespace PDF


