#ifndef PDF_FILE_HPP
#define PDF_FILE_HPP

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
#include "SecHandler.hpp"
#include "ObjStrm.hpp"

namespace PDF {

/** \brief Represents pdf file
 * 
 * Used to access PDF low level file internals, such as xref table and
 * individual objects retrieval.
 *
 */
class File
{
//	friend class Object;
  private:
    std::string filename;
    std::fstream file;
		ObjIStream strm;
    float pdf_version;
    /// references to all root node generations, starting with last one
    std::vector<ObjId> root_refs;
    /// full xref table
    typedef std::map<ObjId,long> XrefTableType;
    XrefTableType xref_table;
    int m_debug;
		SecHandler * m_security;
		std::vector< std::string > m_file_ids;
  protected:
    long offset_lookup(const ObjId & oi) const;
    std::string check_header();
    long get_first_xreftable_offset();
    bool read_xref_table_part(long off);
    Dictionary * read_trailer();
    void load_xref_table();
		void load_crypto_dict(const Dictionary * d);
		void load_file_ids(const Array * a);
  public:
    File(std::string fn="");
    ~File();
		std::string id(unsigned int n = 0) const { return (n >= m_file_ids.size())?"":m_file_ids[0]; }
    /// sets debug level, returns previous debug level
    int debug(int d) { int t=m_debug; m_debug=d; return t; }
    /// returns pdf file version
    float version() const { return pdf_version; }
    bool open(std::string fname="");
    bool close();
    bool load();
    void dump() const;
		SecHandler * security() { return m_security; };
    
    Object * load_object(const ObjId & oi, bool decrypt = true);

    /// returns number of root element references found in file
    long generations_num() const { return root_refs.size(); }
    /// returns root object id for a given generation
    const ObjId & get_root(long generation=0) const { return root_refs[generation]; }
};


}; // namespace PDF



#endif /* PDF_FILE_HPP */

