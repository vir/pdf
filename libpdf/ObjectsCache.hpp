
#ifndef PDF_OBJECTSCACHE_HPP
#define PDF_OBJECTSCACHE_HPP

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <map>
#include "Object.hpp"
//#include "OH.hpp"

namespace PDF {

class File;
class OH;


/// \todo use normal exceptions, not std::string
class ObjectsCache
{
  friend class OH;
  private:
    class CacheEntry
    {
      private:
      public:
        CacheEntry(const CacheEntry & x):obj(x.obj),usecount(x.usecount),is_modified(x.is_modified)
        {
//          std::clog << "CopyConstructing CacheEntry" << this << "(" << &x << ")" << std::endl;
        }
        CacheEntry(Object * o=NULL, unsigned int c=0, bool modified=false):obj(o),usecount(c),is_modified(modified)
        {
//          std::clog << "Constructing CacheEntry" << this << "(" << o->dump() << ")" << std::endl;
        }
        ~CacheEntry()
        {
//          std::clog << "Deleting CacheEntry" << this << std::endl;
        }
        void set_modified() { is_modified = true; }
        Object * obj;
        unsigned long usecount;
        bool is_modified;
    };
    File & m_file;
    std::map<ObjId, CacheEntry> cache;
    typedef std::map<ObjId, CacheEntry>::iterator CacheIterator;
  protected:
    void inc_counter(CacheIterator it)
    {
      it->second.usecount++;
    }
    void dec_counter(CacheIterator it)
    {
      if(it->second.usecount)
        it->second.usecount--;
      if(it->second.usecount == 0 && !it->second.is_modified)
      {
        delete it->second.obj;
        cache.erase(it);
      }
    }
    void inc_counter(const ObjId & id);
    void dec_counter(const ObjId & id);
  public:
    ObjectsCache(File & f):m_file(f)  { }
    ~ObjectsCache()
    {
      // delete all cached objects
      for(CacheIterator it=cache.begin(); it!=cache.end(); it++) delete it->second.obj;
    }
		OH get_object(const ObjId & id);
		OH get_handle(Object * o);
		void flush()
		{
			cache.clear();
		}
		OH new_object(Object * o);
		void save_modified();
		File & file() { return m_file; }
};


} // namespace PDF
#endif /* PDF_OBJECTSCACHE_HPP */




