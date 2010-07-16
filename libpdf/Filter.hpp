
#ifndef PDF_FILTER_H
#define PDF_FILTER_H

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <vector>
#include <string>

namespace PDF {

class Dictionary;

/// general filter interface
class Filter
{
  public:
		virtual ~Filter() {};
		static Filter * Create(std::string name, Dictionary * params = NULL);
    virtual bool Encode(const std::vector<char> & src, std::vector<char> & dst)=0;
    virtual bool Decode(const std::vector<char> & src, std::vector<char> & dst)=0;
		virtual const char * Name()=0;
};

/// LZW and FlateFilter Predictors base class
class PredictorBase
{
	public:
		virtual void Insert(std::vector<char> & data) = 0;
		virtual void Remove(std::vector<char> & data) = 0;
};

/// Zlib wrapper (FlateFilter)
class FlateFilter:public Filter
{
	private:
		PredictorBase * m_predictor;
	public:
		FlateFilter(const Dictionary * params = NULL);
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


