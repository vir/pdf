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
#include "Exceptions.hpp"

namespace PDF {
/** \brief Cross-reference table
 * The cross-reference table contains information that permits random
 * access to indirect objects within the file so that the entire file
 * need not be read to locate any particular object.
 */
class XRefTable
{
public: // types
	struct Entry
	{
		unsigned long offset;
		unsigned int obj_stream_index;
		bool compressed;
		Entry(unsigned long off = 0):offset(off), obj_stream_index(0), compressed(false) { }
		Entry(unsigned long objnum, unsigned int index):offset(objnum), obj_stream_index(index), compressed(true) { }
	};
private:
	std::map<ObjId, Entry> m_table;
public:
	Entry * find(const ObjId & oid)
	{
		std::map<ObjId, Entry>::iterator it = m_table.find(oid);
		return (it != m_table.end())?&it->second:NULL;
	}
	const Entry * find(const ObjId & oid) const
	{
		std::map<ObjId, Entry>::const_iterator it = m_table.find(oid);
		return (it != m_table.end())?&it->second:NULL;
	}
	long get_offset(const ObjId & oid) const
	{
		const Entry * e = find(oid);
		if(!e)
			return 0; // XXX may be throw somthing?
		if(e->compressed)
			throw UnimplementedException("Compressed objects streams");
		return e->offset;
	}
	void dump(std::ostream & strm = std::clog) const
	{
		for(std::map<ObjId, Entry>::const_iterator it = m_table.begin(); it != m_table.end(); ++it) {
			if(it->second.compressed)
				strm << "  object (" << it->first.num << "," << it->first.gen << ") is in stream " << it->second.offset << ", index: " << it->second.obj_stream_index << std::endl;
			else
				strm << "  object (" << it->first.num << "," << it->first.gen << ") is at " << it->second.offset << std::endl;
		}
	}
	void insert_normal(const ObjId & objid, unsigned long offset)
	{
		m_table[objid] = Entry(offset);
	}
	void insert_empty(const ObjId & objid, unsigned long offset)
	{
		// we currently ignore free objects lists
	}
	void insert_compressed(const ObjId & objid, unsigned long stream_id, unsigned int index)
	{
		m_table[objid] = Entry(stream_id, index);
	}
};

class ObjectStreamsCache
{
	private:
		struct Entry
		{
			ObjectStream * s;
			Entry(ObjectStream * strm):s(strm) { }
		};
		std::map<long, Entry> m_stash;
		File & m_file;
	public:
		ObjectStreamsCache(File & f):m_file(f) { }
		Object * load_object(long obj_stream_num, unsigned int obj_stream_index);
};

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
    //typedef std::map<ObjId,long> XrefTableType;
    //XrefTableType xref_table;
	XRefTable xref_table;
    int m_debug;
		SecHandler * m_security;
		std::vector< std::string > m_file_ids;
		ObjectStreamsCache m_streams;
  protected:
    long offset_lookup(const ObjId & oi) const;
    std::string check_header();
    long get_first_xreftable_offset();
    bool read_xref_table_part(long off);
		void read_xref_stream(Stream * s);
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

