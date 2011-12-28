
#include "OH.hpp"
#include "ObjectsCache.hpp"
#include "File.hpp"

namespace PDF {




OH ObjectsCache::get_object(const ObjId & id)
{
  // get from cache or load from file, add to cache and return
  CacheIterator it=cache.find(id);
  if(it == cache.end()) // load
  {
    Object * o=m_file.load_object(id);
    if(!o)
      throw std::exception("Can't load object from file");
    std::pair<CacheIterator, bool> r=cache.insert(std::make_pair<ObjId, CacheEntry>(id, CacheEntry(o)));
    if(!r.second)
      throw std::exception("Can't insert object into cache");
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
		std::pair<CacheIterator, bool> r = cache.insert(std::make_pair<ObjId, CacheEntry>(o->m_id, CacheEntry(o)));
		if(!r.second)
			throw std::exception("Can't insert object into cache");
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


