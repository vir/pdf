
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
    Object * o=file.load_object(id); if(!o) throw std::string("Can't load object");
    std::pair<CacheIterator, bool> r=cache.insert(std::make_pair<ObjId, CacheEntry>(id, CacheEntry(o)));
    if(!r.second) throw std::string("Can't insert object into cache");
    return OH(this, o);
  }
  else // here
  {
    return OH(this, it->second.obj);
  }
}

} // namespace PDF


