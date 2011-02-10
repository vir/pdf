#ifndef FONT_CMAP_HPP_INCLUDED
#define FONT_CMAP_HPP_INCLUDED

#include "Font.hpp"

namespace PDF {

class ObjIStream;
class Font::CMap
{
	private:
		class Range
		{
			private:
				long begin, end;
				wchar_t res;
			public:
				Range(long l1, long l2, wchar_t wc):begin(l1),end(l2),res(wc) {}
				bool load(ObjIStream & s);
				bool yours(long c) const { return(c>=begin && c<=end); }
				wchar_t map(long c) const { return wchar_t(res+(c-begin)); }
		};
		std::map<long,wchar_t> charmap;
		std::vector<Range> charranges;
		unsigned int charbytes;
		unsigned int load_codespacerange(ObjIStream & s);
		bool load_bfchar(ObjIStream & s);
		bool load_bfrange(ObjIStream & s);
	public:
		CMap():charbytes(1) {}
		~CMap() {}
		bool load(ObjIStream & s);
		bool load(OH cmapnode);
		unsigned int cbytes() const { return charbytes; }
		wchar_t map(unsigned long c, bool no_fallback = false) const;
		std::string dump() const
		{
			std::stringstream ss; 
			for(std::map<long,wchar_t>::const_iterator it=charmap.begin(); it!=charmap.end(); it++)
			{
//            ss << "Map " << it->first << " to " << it->second << std::endl;
				char buf[1024];
				sprintf(buf, "Map %04lX to %04X\n", it->first, it->second);
				ss << buf;
			}
			return ss.str();
		}
};

} // namespace PDF

#endif /* FONT_CMAP_HPP_INCLUDED */

