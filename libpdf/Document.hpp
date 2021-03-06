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

    /// object cache
    ObjectsCache cache;
    
    /// Document Catalog dictionary
    OH doc_root;
    
    /// identifiers of all page nodes
    std::vector<OH> all_pages;

		/// next free object id
		ObjId next_free_object_id;
	protected:
		void initialise_new();
		void parse_pages_tree(OH pagenode_h);
		OH build_pages_tree();
		ObjId new_object_id();

  public:
    Document(File & f, int debug=0);
    ~Document();
		void save();
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }

		OH get_page_node(long pagenum) { all_pages[pagenum].expand();  return all_pages[pagenum]; }
		unsigned int get_pages_count() const { return all_pages.size(); }
		OH add_page();

    void dump();
    OH get_object(const ObjId & id);
		OH new_indirect_object(Object * o);
    std::string dump_pages_directory() const
    {
      std::stringstream ss;
      ss << "Pages directory:" << std::endl << "\tPage\tObject" << std::endl;
      for(unsigned int i=0;i<all_pages.size();i++)
      {
        ss << "\t" << i << "\t" << all_pages[i].id().num << "," << all_pages[i].id().gen << std::endl;
      }
      return ss.str();
    }

};


}; // namespace PDF



#endif /* PDF_DOCUMENT_HPP */


