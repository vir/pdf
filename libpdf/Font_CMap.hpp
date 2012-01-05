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
		bool m_identity;
		unsigned int charbytes;
		unsigned int load_codespacerange(ObjIStream & s);
		bool load_bfchar(ObjIStream & s);
		bool load_bfrange(ObjIStream & s);
	public:
		CMap():charbytes(1),m_identity(false) {}
		~CMap() {}
		bool load(ObjIStream & s);
		bool load(const std::string & s);
		bool load(OH cmapnode);
		unsigned int cbytes() const { return charbytes; }
		wchar_t map(unsigned long c, bool no_fallback = false) const;
		std::string dump() const;
};

} // namespace PDF

#endif /* FONT_CMAP_HPP_INCLUDED */

