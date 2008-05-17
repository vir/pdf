#ifndef PDF_FONT_HPP_INCLUDED
#define PDF_FONT_HPP_INCLUDED

#include "OH.hpp"
#include <iostream>
#include <map>

namespace PDF {

/// Represents PDF Font
class Font
{
  private:
    class CMap
    {
      private:
				class Range
				{
					private:
						long begin, end;
						wchar_t res;
					public:
						Range(long l1, long l2, wchar_t wc):begin(l1),end(l2),res(wc) {}
						bool load(std::istream & s);
						bool yours(long c) const { return(c>=begin && c<=end); }
						wchar_t map(long c) const { return res+(c-begin); }
				};
        std::map<long,wchar_t> charmap;
				std::vector<Range> charranges;
				enum { RealMap, IdentityH, WinAnsiEncoding, UnknownEncoding } maptype;
				unsigned int charwidth;
				bool load_bfchar(std::istream & s);
				bool load_bfrange(std::istream & s);
      public:
        CMap():maptype(UnknownEncoding),charwidth(1) {}
        ~CMap() {}
        bool load(std::istream & s);
				bool set_encoding(const std::string & enc);
				unsigned int cbytes() const { return charwidth; }
        inline wchar_t map(unsigned long c) const;
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
    CMap to_unicode_map;
		std::string fontname, fontobjid;
		std::map<int, unsigned long> charwidths;
		unsigned long defcharwidth;
		std::string fonttype, basefont;
		long fontflags;
	protected:
		bool load_type0_font_dic(OH fdic);
  public:
    Font(std::string name="");
    bool load(OH fontnode);
    ~Font();
    wchar_t to_unicode(int c) const;
//    std::string dump() const { return to_unicode_map.dump(); }
    std::string dump() const;
		std::string name() const { return fontname; }
		std::wstring extract_text(const String * so, double * twid = NULL) const;
		bool is_multibyte() const { return to_unicode_map.cbytes()!=1; }
};



}; // naespacs PDF

#endif /* PDF_FONT_HPP_INCLUDED */


