
#ifndef PDF_OBJECT_HPP
#define PDF_OBJECT_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

namespace PDF {

/// Indirect object identifier
class ObjId
{
	public:
		long num, gen;
//		ObjId(long n=0, long g=0):num(n),gen(g) {}
		bool operator ==(const ObjId & other) const { return (num==other.num) && (gen==other.gen); }
		bool operator <(const ObjId & other) const { return (num==other.num)?gen<other.gen:num<other.num; }
		std::string dump() const { std::stringstream ss; ss << "(" << num << "," << gen << ")"; return ss.str(); }
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
    virtual std::string dump(int level=0) const=0;
    virtual std::string dump_objattr() const
    {
      std::stringstream ss;
      if(indirect) ss << "{" << m_id.num << "," << m_id.gen << "}";
      else         ss << "@" << m_offset <<"@";
      return ss.str();
    }
    /// read direct object from given stream
    static Object * read(std::istream & f, bool alt=false);
    /// read indirect object from given stream
    static Object * read_indirect(std::istream & f);
    static unsigned int m_debug;
    /// dirty hack --- returns type name of this object
    std::string type() const;
  protected:
    static Object * read_delimited(std::istream & f, bool alt=false);
};

/// PDF Object: Null --- empty or invalid/unsupported object
class Null:public Object
{
  public:
    Null() { }
    std::string dump(int level=0) const { return "NULL"; }
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
    virtual std::string dump(int level=0) const { std::stringstream ss; ss << my_value; return ss.str(); }
//      virtual std::string out() { return std::string(""); }
};
typedef ObjSimpleT<long> Integer;
typedef ObjSimpleT<double> Real;
/// PDF Object: Boolean data type (true/false)
class Boolean:public ObjSimpleT<bool>
{
  public:
    Boolean(bool b):ObjSimpleT<bool>(b) {}
    virtual std::string dump(int level=0) const { return my_value?"True":"False"; }
};
/// PDF Object: String
class String:public ObjSimpleT<std::string>
{
  public:
    String():ObjSimpleT<std::string>() {}
    String(std::string s):ObjSimpleT<std::string>(s) {}
    virtual std::string dump(int level=0) const { return std::string("(")+my_value+std::string(")"); }
		String * cut_word()
		{
			unsigned int ind = my_value.find(' ');
			if(ind == std::string::npos) return NULL;
			String * r = new String(std::string(my_value, 0, ind));
			my_value.erase(0, ind + 1);
			return r;
		}
};
/// PDF Object: Name --- represents some kind of name
class Name:public ObjSimpleT<std::string>
{
  public:
    Name(std::string n):ObjSimpleT<std::string>(n) {}
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
    std::string dump(int level=0) const
    {
      std::stringstream ss;
      ss << "Array[ " << std::endl;
      for(std::vector<Object *>::const_iterator it=d.begin(); it!= d.end(); it++)
      {
        for(int t=0; t<level; t++) { ss << "\t"; }
        ss << "\t" << (*it)->dump() << std::endl;
      }
      for(int t=0; t<level; t++) { ss << "\t"; }
      ss << "]" << std::endl;
      return ss.str();
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
    virtual std::string dump(int level=0) const
    {
      std::stringstream ss;
      ss << dump_objattr() << "<<" << std::endl;
      for(std::map<std::string, Object *>::const_iterator it=d.begin(); it!=d.end(); it++)
      {
        for(int t=0; t<level; t++) { ss << "\t"; }
        ss << "\t" << it->first << " => " << it->second->dump(level+1) << std::endl;
      }
      for(int t=0; t<level; t++) { ss << "\t"; }
      ss << ">>";// << std::endl;
      return ss.str();
    }
    /// returns member object or null if no such object found
    Object * find(std::string k) const
    {
      std::map<std::string, Object *>::const_iterator it;
      it=d.find(k);
      return (it != d.end())?it->second:NULL;
    }
    
    // iterator functions
    typedef std::map<std::string, Object *>::iterator Iterator;
    Iterator get_iterator() { return d.begin(); }
    bool check_iterator(const Iterator & it) { return it!=d.end(); }
    unsigned long size() const { return d.size(); }
};

class File;

/// PDF Object: Stream --- large/binary data container.
class Stream:public Object
{
/// \todo Make "more valid" stream (see http://www.cplusplus.com/ref/iostream/streambuf/)
  private:
		File * source;
    Dictionary * dict;
		std::istream * file;
		unsigned long soffset;
    std::vector<char> data;
  public:
    Stream(Dictionary * d, std::vector<char> & b):source(NULL),file(NULL) { dict=d; data=b; }
		Stream(Dictionary * d, std::istream * strm, unsigned long offs):source(NULL),dict(d),file(strm),soffset(offs) { }
    virtual ~Stream() { if(dict) delete dict; }
    std::string dump(int level=0) const
    {
			return std::string("Stream: ") + dict->dump();
//      std::stringstream ss;
//      ss << "Stream(" << data.size() << " bytes)";
//      return ss.str();
    }
    bool get_data(std::vector<char> & buf);
    std::string value()
    {
      std::vector<char> v;
      get_data(v);
      return std::string(v.begin(),v.end());
    }
		void set_source(File * f) { source = f; }
};

/// PDF Psewdo-object: Reference to an Indirect object.
class ObjRef:public Object
{
  private:
    ObjId m_ref;
  public:
    ObjRef(long n, long g) { m_ref.num=n; m_ref.gen=g; }
    const ObjId & ref() const { return m_ref; }
    std::string dump(int level=0) const
    {
      std::stringstream ss;
      ss << "ObjRef" << dump_objattr() << "(" << m_ref.num << "," << m_ref.gen << ")";
      return ss.str();
    }
};

/// PDF Psewdo-object: Keyword in some internal structures
class Keyword:public ObjSimpleT<std::string>
{
  public:
    Keyword(std::string s):ObjSimpleT<std::string>(s) {}
    virtual std::string dump(int level=0) const { return std::string("KW_")+my_value; }
};

}; // namespace PDF




#endif /* PDF_OBJECT_HPP */

