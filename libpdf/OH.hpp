
#ifndef PDF_OH_HPP
#define PDF_OH_HPP
//#include "Document.hpp"
#include <map>
#include "Object.hpp"
#include "ObjectsCache.hpp"

namespace PDF {

class File;

/// Smart pointer to Document's Object (Object Handle)
class OH
{
  friend class ObjectsCache;
  private:
    /// Pointer to an ObjectsCache, from which this OH was born
    ObjectsCache * m_cache;
    /// Pointer to an Object
    Object * m_ptr;
  protected:
    /// Protected constructor (used by ObjectsCache and dig())
    OH(ObjectsCache * c, Object * o):m_cache(c),m_ptr(o) { if(o && o->indirect) c->inc_counter(o->m_id); }
  public:
    /// Default constructor
    OH():m_cache(NULL),m_ptr(NULL) {}
    /// Copy constructor
    OH(const OH & h):m_cache(h.m_cache),m_ptr(h.m_ptr) { if(m_cache && m_ptr && m_ptr->indirect) m_cache->inc_counter(m_ptr->m_id); }
    /// Destructor
    ~OH() { if(m_cache && m_ptr && m_ptr->indirect) m_cache->dec_counter(m_ptr->m_id); }
    /// Assignment operator
    OH & operator=(const OH & z)
    {
      if(m_ptr && m_cache && m_ptr->indirect) m_cache->dec_counter(m_ptr->m_id);
      m_cache=z.m_cache;
      m_ptr=z.m_ptr;
      if(m_ptr && m_cache && m_ptr->indirect) m_cache->inc_counter(m_ptr->m_id);
      return *this;
    }
    
    /// check for error
    bool operator!() { return !m_ptr; }
    /// check for ok
    operator bool() { return m_ptr!=NULL; }
    /// Object class members access
    Object * operator->() const throw() { return m_ptr; }
    /// dynamic cast to type T and throw exception errstr if unable to cast
    template<typename T> T cast(std::string errstr="") const
    {
      T r=dynamic_cast<T>(m_ptr);
      if(!errstr.empty() && !r) throw errstr;
      return r;
    }
    /// returns handle to child object
    /// \todo XXX remove it asap! XXX
    OH dig(Object * o) const { return OH(m_cache, o); }
    /// returns object id
    ObjId id() const { return m_ptr->indirect?m_ptr->m_id:ObjId(); }
    /// returns Object pointer
    Object * obj() { return m_ptr; }
    const Object * obj() const { return m_ptr; }
    
    //-------- high-level objects access
    /// Returns size of array/dictionary
    unsigned long size() const
    {
      Array * a=cast<Array*>();
      Dictionary * d=cast<Dictionary*>();
      // XXX check streams?
      if(a) return a->size();
      else if(d) return d->size();
      else throw std::string("Not a container --- no size()");
    }
    /// Find object in dictionary
    OH find(const std::string & s) const
    {
      Dictionary * d=cast<Dictionary*>("Can't 'find' - not a dictionary");
      return OH(m_cache, d->find(s));
    }
    /// Get array element by it's index
    OH operator[](int index)
    {
      Array * a=cast<Array*>("Can't [index] - not an array");
      return OH(m_cache, a->at(index));
    }
		/// Get name or string content
		std::string strvalue() const
		{
			const String * s = cast<const String *>();
			if(s) return s->value();
			const Name * n = cast<const Name *>(std::string("can't extract string value from ") + obj()->type());
			return n->value();
		}
    /// Replace indirect object reference with real object
    bool expand()
    {
      ObjRef * ref=dynamic_cast<ObjRef *>(m_ptr);
      if(ref)
      {
        ObjId id=ref->ref();
        *this=m_cache->get_object(id);
        return true;
      }
      else return false;
    }
};




} // namespace PDF
#endif /* PDF_OH_HPP */




