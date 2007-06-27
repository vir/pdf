#include "Filter.hpp"
#include <string>

namespace PDF {

Filter * Filter::Create(std::string name)
{
  if(name == "FlateDecode") { return new FlateFilter(); }
	if(name == "ASCII85Decode") { return new Base85Filter(); }
	return NULL;
}

} // namespace PDF



