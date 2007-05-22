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
				bool Font::CMap::load_bfchar(std::istream & s);
				bool Font::CMap::load_bfrange(std::istream & s);
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
		std::string fontname;
  public:
    Font(std::string name="");
    bool load(OH fontnode);
    ~Font();
    wchar_t to_unicode(int c) const;
    std::string dump() const { return to_unicode_map.dump(); }
		std::string name() const { return fontname; }
		std::wstring Font::extract_text(const String * so) const;
};



}; // naespacs PDF


