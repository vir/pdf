#ifndef FONT_ENCODING_HPP_INCLUDED
#define FONT_ENCODING_HPP_INCLUDED

#include "Font.hpp"

namespace PDF {

/** Font encoding abstraction */
class Font::Encoding
{
	public:
		enum Type { UnknownEncoding, WinAnsiEncoding, MacRomanEncoding, MacExpertEncoding, /*RealMap,*/ IdentityH };
	private:
		enum Type enc;
		std::map<long,wchar_t> charmap;
	public:
		Encoding():enc(UnknownEncoding) {}
		const char * name() const;
		void load(OH encnode);
		bool set_encoding(const std::string & n);
		inline void add_diff(long l, wchar_t c) { charmap[l] = c; }
		wchar_t map(unsigned long c) const;
		bool is_singlebyte() const
		{
			switch(enc) {
				case WinAnsiEncoding: case MacRomanEncoding: case MacExpertEncoding: return true;
				default: return false;
			}
		}
};

} // namespace PDF

#endif /* FONT_ENCODING_HPP_INCLUDED */

