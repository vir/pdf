
#ifndef PDF_OBJECT_HPP
#define PDF_OBJECT_HPP

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <vector>
#include <sstream>

namespace PDF {

/// Indirect object identifier
class ObjId
{
	static const long gen_wildcard = 65535;
	public:
		long num, gen;
//		ObjId(long n=0, long g=0):num(n),gen(g) {}
		bool operator ==(const ObjId & other) const
		{
			return (num==other.num)
				&& ((gen==other.gen) || gen == gen_wildcard || other.gen == gen_wildcard);
		}
		bool operator <(const ObjId & other) const
		{
			if(num==other.num)
				return (!(gen == gen_wildcard || other.gen == gen_wildcard))
					&& gen<other.gen;
			else
				return num<other.num;
		}
		void dump(std::ostream & ss) const { ss << "(" << (int)num << "," << (int)gen << ")"; }
		std::string dump() const { std::stringstream ss; dump(ss); return ss.str(); }
};

/// Generalized PDF object interface and common attributes
class Object
{
  public:
    /// Object is an Indirect Object
    bool indirect;
    // save 4 bytes/obj memory... :)
    union {
      /// Object id for indirect object
      ObjId m_id;
      /// File offset for direct objects
      long m_offset;
    };

    // === methods ===

    Object() { m_id.num=0; m_id.gen=0; indirect=false; }
    virtual ~Object() {};
    /// returns string, somehow desribing object's content.
		virtual void dump(std::ostream & ss, int level=0) const=0;
    virtual std::string dump(int level=0) const { std::stringstream ss; dump(ss, level); return ss.str(); }
    virtual std::string dump_objattr() const
    {
      std::stringstream ss;
      if(indirect) m_id.dump(ss);
      else ss << "@" << m_offset <<"@";
      return ss.str();
    }
    static unsigned int m_debug;
    /// dirty hack --- returns type name of this object
    std::string type() const;
	template<typename T> bool to_number(T& rret) const;
  protected:
    static Object * read_delimited(std::istream & f, bool alt=false);
};

/// PDF Object: Null --- empty or invalid/unsupported object
class Null:public Object
{
  public:
    Null() { }
		virtual void dump(std::ostream & ss, int level=0) const { ss << "NULL"; }
    virtual std::string dump(int level=0) const { return "NULL"; }
};

/// template for simple types (Bool, Integer, Real and String) generation.
template <class T> class ObjSimpleT:public Object
{
  protected:
    T my_value;
  public:
    ObjSimpleT() { }
    ObjSimpleT(T v):my_value(v) { }
    virtual ~ObjSimpleT() {}
    virtual T value() const { return my_value; }
		virtual void dump(std::ostream & ss, int level=0) const { ss << my_value; }
//      virtual std::string out() { return std::string(""); }
};
typedef ObjSimpleT<long> Integer;
typedef ObjSimpleT<double> Real;
/// PDF Object: Boolean data type (true/false)
class Boolean:public ObjSimpleT<bool>
{
  public:
    Boolean(bool b):ObjSimpleT<bool>(b) {}
		virtual void dump(std::ostream & ss, int level=0) const { ss << (my_value?"True":"False"); }
    virtual std::string dump(int level=0) const { return my_value?"True":"False"; }
};
/// PDF Object: String
class String:public Object
{
	private:
		std::vector<char> my_value;
  public:
    String() {}
    String(std::vector<char> s):my_value(s) {}
		virtual void dump(std::ostream & ss, int level=0) const
		{
			unsigned int pos;
			ss << '(';
			for(pos = 0; pos < my_value.size(); pos++) {
				unsigned int c = (unsigned int)(unsigned char)my_value[pos];
				if(c < 0x20 || c >= 0x7F)
					ss << "\\x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << c;
				else
					ss << (char)c;
			}
			ss << ')';
		}
		std::string str() const
		{
			return std::string(&my_value[0], my_value.size());
		}
		std::vector<char> value() { return my_value; }
		const std::vector<char> value() const { return my_value; }
};
/// PDF Object: Name --- represents some kind of name
class Name:public ObjSimpleT<std::string>
{
  public:
    Name(std::string n):ObjSimpleT<std::string>(n) {}
		virtual void dump(std::ostream & ss, int level=0) const { ss << "/" << my_value; }
    virtual std::string dump(int level=0) const { return std::string("/")+my_value; }
};
/// PDF Object: Array --- collection of other objects
class Array:public Object
{
  public:
    typedef std::vector<Object *>::const_iterator ConstIterator;
  private:
    std::vector<Object *> d;
  public:
    Array() { }
    virtual ~Array()
    {
      for(std::vector<Object *>::iterator it=d.begin(); it!=d.end(); it++)
      {
        delete *it;
      }
    }
    void push(Object * o) { d.push_back(o); }
		virtual void dump(std::ostream & ss, int level=0) const
    {
      ss << "Array[ " << std::endl;
      for(std::vector<Object *>::const_iterator it=d.begin(); it!= d.end(); it++)
      {
        for(int t=0; t<level; t++) { ss << "\t"; }
        ss << "\t";
				(*it)->dump(ss, level+1);
				ss << std::endl;
      }
      for(int t=0; t<level; t++) { ss << "\t"; }
      ss << "]" << std::endl;
    }
    ConstIterator get_const_iterator() const { return d.begin(); }
    bool check_iterator(const ConstIterator & it) const { return it!=d.end(); }
    unsigned long size() const { return d.size(); }
    Object * at(unsigned long i) { return d.at(i); }
    const Object * at(unsigned long i) const { return d.at(i); }
};

/// PDF Object: Map --- maps Names to other Objects.
class Dictionary:public Object
{
  private:
    std::map<std::string, Object *> d;
  public:
    Dictionary() { }
    /// \todo rewrite it using foreach
    virtual ~Dictionary()
    {
      for(std::map<std::string, Object *>::const_iterator it=d.begin(); it!=d.end(); it++)
      {
        //delete it->first;
        delete it->second;
      }
    }
    /// adds (replaces) mapping
    void set(std::string /*Name * */k, Object * v) { d[k]=v; }
		virtual void dump(std::ostream & ss, int level=0) const
    {
      ss << dump_objattr() << "<<" << std::endl;
      for(std::map<std::string, Object *>::const_iterator it=d.begin(); it!=d.end(); it++)
      {
        for(int t=0; t<level; t++) { ss << "\t"; }
        ss << "\t" << it->first << " => ";
				it->second->dump(ss, level+1);
				ss << std::endl;
      }
      for(int t=0; t<level; t++) { ss << "\t"; }
      ss << ">>";// << std::endl;
    }
    /// returns member object or null if no such object found
    Object * find(std::string k) const
    {
      std::map<std::string, Object *>::const_iterator it;
      it=d.find(k);
      return (it != d.end())?it->second:NULL;
    }
		/// sortcut for dynamic_cast<...>(find(...))
		template<typename T>
		bool find(std::string k, T *& pp) const
		{
			Object * o = find(k);
			if(!o)
				return false;
			pp = dynamic_cast<T *>(o);
			return !!pp;
		}
    
    // iterator functions
    typedef std::map<std::string, Object *>::iterator Iterator;
    Iterator get_iterator() { return d.begin(); }
    bool check_iterator(const Iterator & it) { return it!=d.end(); }
    unsigned long size() const { return d.size(); }
};

class ObjRef;
class File;
class ObjIStream;
class ObjOStream;

/// PDF Object: Stream --- large/binary data container.
class Stream:public Object // All streams must be indirect objects
{
/// \todo Make "more valid" stream (see http://www.cplusplus.com/ref/iostream/streambuf/)
	private:
		File * source;
		Dictionary * m_dict;
		ObjIStream * ostrm;
		unsigned long soffset;
		mutable ObjRef * m_slength_ref;
		mutable unsigned long m_slength;
		std::vector<char> m_data;
		bool m_encryption;
	public:
		Stream(Dictionary * d, std::vector<char> & b): source(NULL),m_dict(d), ostrm(NULL), soffset(0), m_slength_ref(NULL), m_slength(0), m_data(b), m_encryption(true) { }
		Stream(Dictionary * d, ObjIStream * s, unsigned int offs, unsigned long length): source(NULL),m_dict(d), ostrm(s), soffset(offs), m_slength_ref(NULL), m_slength(length), m_encryption(true) { }
		Stream(Dictionary * d, ObjIStream * s, unsigned int offs, ObjRef * length): source(NULL),m_dict(d), ostrm(s), soffset(offs), m_slength_ref(length),m_slength(0), m_encryption(true) { }
		Stream(bool encrypt = true): source(NULL),m_dict(new Dictionary),ostrm(NULL), soffset(0), m_slength_ref(NULL), m_slength(0), m_encryption(encrypt) { }
		virtual ~Stream() { if(m_dict) delete m_dict; }
		virtual void dump(std::ostream & ss, int level=0) const
		{
			ss << "Stream: " << slength() << " bytes at offset " << soffset << " ";
			m_dict->dump(ss);
		}

		const Dictionary * dict() const { return m_dict; }
		Dictionary * dict() { return m_dict; }

		bool encryption() const { return m_encryption; }
		void encryption(bool enable) { m_encryption = enable; }

		bool get_data(std::vector<char> & buf);

		void put_data(const std::vector<char> & buf);
		void save_data(ObjOStream * ostrm);
		std::string value()
		{
			std::vector<char> v;
			get_data(v);
			return std::string(v.begin(),v.end());
		}
		void set_source(File * f) { source = f; }
		unsigned int slength() const;
		bool delayed_load() const { return m_slength_ref != NULL; }
};

/// PDF Psewdo-object: Reference to an Indirect object.
class ObjRef:public Object
{
  private:
    ObjId m_ref;
  public:
    ObjRef(long n, long g) { m_ref.num=n; m_ref.gen=g; }
    ObjRef(ObjId id) { m_ref = id; }
    const ObjId & ref() const { return m_ref; }
		virtual void dump(std::ostream & ss, int level=0) const
    {
      ss << "ObjRef" << dump_objattr() << "(" << m_ref.num << "," << m_ref.gen << ")";
    }
};

/// Placeholder for an free object identifier. Should not appear if all is good.
class FreeObjectPlaceholder:public Object
{
public:
	FreeObjectPlaceholder() { }
	virtual void dump(std::ostream & ss, int level=0) const { ss << "FreeObject" << dump_objattr(); }
};

/// Placeholder for a missing object. May appear in totally broken PDF only.
class MissingObjectPlaceholder:public Object
{
public:
	MissingObjectPlaceholder() { }
	virtual void dump(std::ostream & ss, int level=0) const { ss << "MissingObject" << dump_objattr(); }
};

/// PDF Psewdo-object: Keyword in some internal structures
class Keyword:public ObjSimpleT<std::string>
{
  public:
    Keyword(std::string s):ObjSimpleT<std::string>(s) {}
		virtual void dump(std::ostream & ss, int level=0) const { ss << "SW_" << my_value; }
    virtual std::string dump(int level=0) const { return std::string("KW_")+my_value; }
};

template<typename T> bool Object::to_number(T& rret) const
{
	const Integer * i = dynamic_cast<const Integer *>(this);
	const Real * r = dynamic_cast<const Real *>(this);
	if(i) { rret = (T)i->value(); return true; }
	if(r) { rret = (T)r->value(); return true; }
	return false;
}


}; // namespace PDF




#endif /* PDF_OBJECT_HPP */

