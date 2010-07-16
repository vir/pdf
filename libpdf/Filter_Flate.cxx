
#include <zlib.h>
#include <vector>
#include <string>
#include "Exceptions.hpp"
#include "Filter.hpp"
#include "Object.hpp"

namespace PDF {

class PredictorPNG_Up:public PredictorBase
{
	private:
		unsigned int m_ncols;
	public:
		PredictorPNG_Up(unsigned int ncols):m_ncols(ncols) { }
		virtual void Insert(std::vector<char> & data);
		virtual void Remove(std::vector<char> & data);
};

void PredictorPNG_Up::Insert(std::vector<char> & data)
{
	throw UnimplementedException("PNG predictor insertion");
}

void PredictorPNG_Up::Remove(std::vector<char> & data)
{
	std::vector<char>::const_iterator r = data.begin();
	std::vector<char>::iterator w = data.begin();
	std::vector<char> prev_row(m_ncols, 0);
	int col = -1;
	while(r != data.end()) {
		if(col == -1) {
			if(*r != 2)
				throw FormatException("Wrong predictor");
		} else {
			*w = prev_row[col] = *r + prev_row[col];
			++w;
		}
		++r;
		++col;
		if((unsigned int)col >= m_ncols)
			col = -1;
	}
	data.resize(w - data.begin());
}

/// Flate filter constructor
/**
 * \param params optional pointer to a parameters dictionaty
 */
FlateFilter::FlateFilter(const Dictionary * params)
 : m_predictor(0)
{
	long predictor_id;
	unsigned int cols;
	if(!params)
		return;
	const Object * o = params->find("Predictor");
	if(o && o->to_number(predictor_id)) {
		switch(predictor_id) {
			case 12:
				if((o = params->find("Columns")) && o->to_number(cols))
					m_predictor = new PredictorPNG_Up(cols);
				else
					throw FormatException("PNG Up predictor: required parameter 'Columns' not found in xref stream dictionary");
				break;
			default:
				throw UnimplementedException("Only PNG Up predictor is implemented :(");
		}
	}
}

/// Calls zlib's deflate(...) to compress data.
/**
 * \todo Implement it!
 * \param src should contain data to be compressed
 * \param dst receives compressed data
 */
bool FlateFilter::Encode(const std::vector<char> & src, std::vector<char> & dst)
{
  return false;
}

/// Calls zlib's inflate(...) to decompress input data.
/**
 * \todo Calculate out buffer more intellectually (based on compression ratio)
 * \param src should contain compressed data block
 * \param dst receives decompressed data
 */
bool FlateFilter::Decode(const std::vector<char> & src, std::vector<char> & dst)
{
  int r;
  z_stream zs;
  zs.next_in=reinterpret_cast<Bytef *>(const_cast<char *>(&src[0]));
  zs.avail_in=src.size();
  zs.zalloc=NULL;
  zs.zfree=NULL;
  
	try {
//  r=inflateInit(&zs);
		r=inflateInit2(&zs, 47);
		if(r!=Z_OK)
			throw std::string("Zlib error: ") + zs.msg;

		int outbufsize=3*src.size();
		int pos=0;
		do {
			dst.resize(outbufsize);
			zs.next_out=reinterpret_cast<Bytef *>(&dst[pos]);
			zs.avail_out=dst.size()-pos;

			r=inflate(&zs, 0);
			pos=outbufsize;
			outbufsize+=src.size(); // increment output buffer size XXX
		} while(r==Z_OK);

		if(r!=Z_STREAM_END)
			throw std::string("Zlib error: ") + zs.msg;
		dst.resize((char*)zs.next_out-&dst[0]); // adjust output object size
		r=inflateEnd(&zs);
		if(r!=Z_OK)
			throw std::string("Zlib error: ") + zs.msg;
		if(m_predictor)
			m_predictor->Remove(dst);
	}
	catch(...) {
		inflateEnd(&zs);
		throw;
	}
  
  return true;
}


}; // namespace PDF

