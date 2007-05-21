
#ifndef PDF_FILTER_H
#define PDF_FILTER_H

#include <vector>

namespace PDF {

/// general filter interface
class Filter
{
  public:
		virtual ~Filter() {};
    virtual bool Encode(const std::vector<char> & src, std::vector<char> & dst)=0;
    virtual bool Decode(const std::vector<char> & src, std::vector<char> & dst)=0;
};

/// Zlib wrapper (FlateFilter)
class FlateFilter:public Filter
{
  public:
		virtual ~FlateFilter() {};
    virtual bool Encode(const std::vector<char> & src, std::vector<char> & dst);
    virtual bool Decode(const std::vector<char> & src, std::vector<char> & dst);
};

}; // namespace PDF

#endif /* PDF_FILTER_H */


