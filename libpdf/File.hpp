#ifndef PDF_FILE_HPP
#define PDF_FILE_HPP

#include <fstream>
#include <string>
#include <map>
#include <vector>
//#include <sstream>

#include "Object.hpp"

namespace PDF {

/** \brief Represents pdf file
 * 
 * Used to access PDF low level file internals, such as xref table and
 * individual objects retrieval.
 *
 */
class File
{
	friend class Object;
  private:
    std::string filename;
    std::fstream file;
    float pdf_version;
    /// references to all root node generations, starting with last one
    std::vector<ObjId> root_refs;
    /// full xref table
    typedef std::map<ObjId,long> XrefTableType;
    XrefTableType xref_table;
    int m_debug;
  protected:
    long offset_lookup(const ObjId & oi) const;
    std::string check_header();
    long get_first_xreftable_offset();
    bool read_xref_table_part(long off);
    Dictionary * read_trailer();
    void load_xref_table();
  public:
    File(std::string fn="");
    ~File();
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }
    /// returns pdf file version
    float version() const { return pdf_version; }
    bool open(std::string fname="");
    bool close();
    bool load();
    void dump() const;
    
    Object * load_object(const ObjId & oi);

    /// returns number of root element references found in file
    long generations_num() const { return root_refs.size(); }
    /// returns root object id for a given generation
    const ObjId & get_root(long generation=0) const { return root_refs[generation]; }
};


}; // namespace PDF



#endif /* PDF_FILE_HPP */

