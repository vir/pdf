
#ifndef PDF_FILTER_H
#define PDF_FILTER_H

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <vector>
#include <string>

class AES_CBC;

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
		virtual ~PredictorBase() { }
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
		virtual ~FlateFilter() { delete m_predictor; };
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

/// AES-CBC encoder/decoder
class AESV2Filter:public Filter
{
	const static unsigned int keysize = 128 / 8;
	const static unsigned int blocksize = 128 / 8;
private:
	AES_CBC * aes;
	char buf[keysize];
	unsigned long bufused;
	std::string key;
protected:
	void process(const std::vector<char> &src, bool encr, std::vector<char> &dst);
	void process(const char * data, bool encr, std::vector<char> &dst);
public:
	AESV2Filter(const Dictionary * params = NULL);
	AESV2Filter(const std::string& key): aes(NULL), bufused(0), key(key) { }
	virtual ~AESV2Filter();
	virtual bool Encode(const std::vector<char> & src, std::vector<char> & dst);

	virtual bool Decode(const std::vector<char> & src, std::vector<char> & dst);
	virtual const char * Name() { return "AESV2Filter"; };
};


}; // namespace PDF

#endif /* PDF_FILTER_H */


