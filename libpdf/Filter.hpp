
#ifndef PDF_FILTER_H
#define PDF_FILTER_H

#include <vector>

namespace PDF {

/// general filter interface
class Filter
{
  public:
		virtual ~Filter() {};
		static Filter * Create(std::string name);
    virtual bool Encode(const std::vector<char> & src, std::vector<char> & dst)=0;
    virtual bool Decode(const std::vector<char> & src, std::vector<char> & dst)=0;
		virtual const char * Name()=0;
};

/// Zlib wrapper (FlateFilter)
class FlateFilter:public Filter
{
  public:
		virtual ~FlateFilter() {};
    virtual bool Encode(const std::vector<char> & src, std::vector<char> & dst);
    virtual bool Decode(const std::vector<char> & src, std::vector<char> & dst);
		virtual const char * Name() { return "FlateFilter"; };
};

/// Base85 converter
class Base85Filter:public Filter
{
  public:
		virtual ~Base85Filter() {};
    virtual bool Encode(const std::vector<char> & src, std::vector<char> & dst);
    virtual bool Decode(const std::vector<char> & src, std::vector<char> & dst);
		virtual const char * Name() { return "Base85Filter"; };
};

}; // namespace PDF

#endif /* PDF_FILTER_H */


