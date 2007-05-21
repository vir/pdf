
#ifndef PDF_OBJECTSCACHE_HPP
#define PDF_OBJECTSCACHE_HPP
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
        CacheEntry(const CacheEntry & x):obj(x.obj),usecount(x.usecount)
        {
//          std::clog << "CopyConstructing CacheEntry" << this << "(" << &x << ")" << std::endl;
        }
        CacheEntry(Object * o=NULL, unsigned int c=0):obj(o),usecount(c)
        {
//          std::clog << "Constructing CacheEntry" << this << "(" << o->dump() << ")" << std::endl;
        }
        ~CacheEntry()
        {
//          std::clog << "Deleting CacheEntry" << this << std::endl;
        }
        Object * obj;
        unsigned long usecount;
    };
    File & file;
    std::map<ObjId, CacheEntry> cache;
    typedef std::map<ObjId, CacheEntry>::iterator CacheIterator;
  protected:
    void inc_counter(CacheIterator it)
    {
      it->second.usecount++;
    }
    void dec_counter(CacheIterator it)
    {
      it->second.usecount--;
      if(it->second.usecount == 0)
      {
        delete it->second.obj;
        cache.erase(it);
      }
    }
    void inc_counter(const ObjId & id)
    {
      CacheIterator it=cache.find(id);
      if(it==cache.end()) throw std::string("Object not in cache");
      else inc_counter(it);
    }
    void dec_counter(const ObjId & id)
    {
      CacheIterator it=cache.find(id);
      if(it==cache.end()) throw std::string("Object not in cache");
      else dec_counter(it);
    }
  public:
    ObjectsCache(File & f):file(f)  { }
    ~ObjectsCache()
    {
      // delete all cached objects
      for(CacheIterator it=cache.begin(); it!=cache.end(); it++) delete it->second.obj;
    }
    OH get_object(const ObjId & id);
};


} // namespace PDF
#endif /* PDF_OBJECTSCACHE_HPP */




