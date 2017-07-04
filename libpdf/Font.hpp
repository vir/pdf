#ifndef PDF_FONT_HPP_INCLUDED
#define PDF_FONT_HPP_INCLUDED

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include "OH.hpp"
#include <iostream>
#include <map>
#include <limits.h>

namespace PDF {

/// Represents PDF Font
class Font
{
	public:
		class Encoding;
		class CMap;
		class Metrics;
	private:
		Encoding * encoding;
		CMap * to_unicode_map;
		Metrics * metrics;
		std::string fontname, fontobjid;
		std::string fonttype, basefont;
#if ULONG_MAX == 4294967295UL // long is 32bit
		long fontflags;
#elif UINT_MAX == 4294967295UL // int is 32bit
		int fontflags;
#elif USHRT_MAX == 4294967295UL // short is 32bit
		short fontflags;
#else
# error Can't determine size of 32bit int!
#endif
		unsigned int charbytes;
	protected:
		bool load_type0_font_dic(OH fdic);
  public:
    Font(std::string name="");
    ~Font();
    bool load(OH fontnode);
    wchar_t to_unicode(int c) const;
    std::string dump() const;
		std::string name() const { return fontname; }
		bool extract_text(const String * so, std::wstring & ws, double & twid, unsigned int & pos, wchar_t delimiter=L'\0') const;
		bool extract_one_char(const String * so, wchar_t & nextchar, double & charwidth, unsigned int & pos) const;
		inline bool is_multibyte() const { return charbytes > 1; }
};

}; // naespacs PDF

#endif /* PDF_FONT_HPP_INCLUDED */


