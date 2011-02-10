#ifndef PDF_FONT_HPP_INCLUDED
#define PDF_FONT_HPP_INCLUDED

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include "OH.hpp"
#include <iostream>
#include <map>

namespace PDF {

/// Represents PDF Font
class Font
{
	public:
		class Encoding;
    class CMap;
  private:
		Encoding * encoding;
    CMap * to_unicode_map;
		std::string fontname, fontobjid;
		std::map<int, unsigned long> charwidths;
		unsigned long defcharwidth;
		std::string fonttype, basefont;
		long fontflags;
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


