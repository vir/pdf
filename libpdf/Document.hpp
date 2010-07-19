#ifndef PDF_DOCUMENT_HPP
#define PDF_DOCUMENT_HPP

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <fstream>
#include <string>
#include <map>
#include <vector>
//#include <sstream>

#include "Object.hpp"

#include "File.hpp"
#include "OH.hpp"


namespace PDF {

//class OH;
//class ObjectsCache;

/// Represents document structure and content
class Document
{
  private:
    int m_debug;

    /// pointer to the object cache
    ObjectsCache cache;
    
    /// Document Catalog dictionary
    OH doc_root;
    
    /// identifiers of all page nodes
    std::vector<ObjId> all_pages;

  protected:
    void parse_pages_tree(OH pagenode_h);

  public:
//    Document(ObjectsCache * c, ObjId root_element, int debug=0);
    Document(File & f, int debug=0);
    ~Document();
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }
		const ObjId & get_page_objid(long pagenum) { return all_pages[pagenum]; }
    OH get_page_node(long pagenum) { return get_object(get_page_objid(pagenum)); }
    unsigned int get_pages_count() const { return all_pages.size(); }
    void dump();
    OH get_object(const ObjId & id);
    std::string dump_pages_directory() const
    {
      std::stringstream ss;
      ss << "Pages directory:" << std::endl << "\tPage\tObject" << std::endl;
      for(unsigned int i=0;i<all_pages.size();i++)
      {
        ss << "\t" << i << "\t" << all_pages[i].num << "," << all_pages[i].gen << std::endl;
      }
      return ss.str();
    }

};


}; // namespace PDF



#endif /* PDF_DOCUMENT_HPP */


