#ifdef _MSC_VER
# pragma warning(disable : 4786) // get rid of "identifier was truncated to '255' characters..."
# ifdef _DEBUG
#  ifdef _CRTDBG_MAP_ALLOC
#   include <stdlib.h>  
#   include <crtdbg.h>  
#   ifndef DBG_NEW
#    define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#    define new DBG_NEW
#   endif
#  endif
# endif // _DEBUG
#endif

#include "OH.hpp"
#include "ObjectsCache.hpp"
#include "File.hpp"

namespace PDF {


void ObjectsCache::inc_counter(const ObjId & id)
{
  CacheIterator it=cache.find(id);
  if(it==cache.end()) throw LogicException("Object not in cache");
  else inc_counter(it);
}

void ObjectsCache::dec_counter(const ObjId & id)
{
  CacheIterator it=cache.find(id);
  if(it==cache.end()) throw LogicException("Object not in cache");
  else dec_counter(it);
}

OH ObjectsCache::get_object(const ObjId & id)
{
  // get from cache or load from file, add to cache and return
  CacheIterator it=cache.find(id);
  if(it == cache.end()) // load
  {
    Object * o=m_file.load_object(id);
    if(!o)
      throw std::runtime_error("Can't load object from file");
    std::pair<CacheIterator, bool> r=cache.insert(std::make_pair(id, CacheEntry(o)));
    if(!r.second)
      throw std::runtime_error("Can't insert object into cache");
    return OH(this, o);
  }
  else // here
  {
    return OH(this, it->second.obj);
  }
}

OH ObjectsCache::get_handle(Object * o)
{
	if(o->indirect) {
		return get_object(o->m_id);
	} else {
		return OH(this, o);
	}
}

PDF::OH ObjectsCache::new_object( Object * o )
{
	if(o->indirect) {
		std::pair<CacheIterator, bool> r = cache.insert(std::make_pair(o->m_id, CacheEntry(o)));
		if(!r.second)
			throw std::runtime_error("Can't insert object into cache");
		return OH(this, o);
	} else
		return get_handle(o);
}

void ObjectsCache::save_modified()
{
	for(CacheIterator it = cache.begin(); it != cache.end(); ++it) {
		if(it->second.is_modified) {
			m_file.save_object(it->second.obj);
			it->second.is_modified = false;
		}
	}
}

} // namespace PDF


