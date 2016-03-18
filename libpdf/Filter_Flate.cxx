
#include <zlib.h>
#include <vector>
#include <string>
#include "Exceptions.hpp"
#include "Filter.hpp"
#include "Object.hpp"

namespace PDF {

class PredictorPNG:public PredictorBase
{
public:
	typedef std::vector<char> DataVector;
private:
	unsigned int m_bpc, m_colors, m_ncols;
	unsigned int m_bpp, m_bpr;
	DataVector m_prev_row;
public:
	PredictorPNG(unsigned int ncols, unsigned int colors, unsigned int bpc)
		: m_ncols(ncols), m_colors(colors), m_bpc(bpc)
	{
		m_bpp = 1 + ((m_bpc * m_colors - 1) / 8); // bytes per point
		m_bpr = m_bpp * m_ncols;
	}
	virtual void Insert(DataVector & data);
	virtual void Remove(DataVector & data);
private:
	inline void RemoveRow_None(DataVector::iterator& w, DataVector::const_iterator& r)
	{
		std::copy(r, r + m_ncols, w);
		r += m_ncols;
		w += m_ncols;
	}
	void RemoveRow_Sub(DataVector::iterator& w, DataVector::const_iterator& r);
	void RemoveRow_Up(DataVector::iterator& w, DataVector::const_iterator& r);
};

void PredictorPNG::Insert(std::vector<char> & data)
{
	throw UnimplementedException("PNG predictor insertion");
}

void PredictorPNG::Remove(std::vector<char> & data)
{
	DataVector::const_iterator r = data.begin();
	DataVector::iterator w = data.begin();
	m_prev_row.resize(m_bpr, 0);
	int col = -1;
	while(r != data.end()) {
		int predictor = *r++;
		if (data.end() - r < m_bpr)
			throw FormatException("Short data row");
		DataVector::iterator rowstart = w;
		switch (predictor) {
		case 0:
			RemoveRow_None(w, r);
			break;
		case 1:
			RemoveRow_Sub(w, r);
			break;
		case 2:
			RemoveRow_Up(w, r);
			break;
		default:
			throw UnimplementedException("Wrong predictor ") << predictor;
			// more predictors here: https://tools.ietf.org/html/rfc2083
		}
		m_prev_row.assign(rowstart, w);
	}
	data.resize(w - data.begin());
}

void PredictorPNG::RemoveRow_Sub(DataVector::iterator& w, DataVector::const_iterator& r)
{
	unsigned int index = 0;
	for (; index < m_bpr; ++index) {
		*w = *r + (index < m_bpp ? 0 : w[-m_bpp]);
		++w; ++r;
	}
}

void PredictorPNG::RemoveRow_Up(DataVector::iterator& w, DataVector::const_iterator& r)
{
	unsigned int b = 0;
	for (; b < m_bpr; ++b) {
		*w = *r + m_prev_row[b];
		++w; ++r;
	}
}

/// Flate filter constructor
/**
 * \param params optional pointer to a parameters dictionaty
 */
FlateFilter::FlateFilter(const Dictionary * params)
 : m_predictor(0)
{
	long predictor_id;
	unsigned int cols, colors, bpc;
	if(!params)
		return;
	const Object * o = params->find("Predictor");
	if(o && o->to_number(predictor_id)) {
		if (predictor_id == 1) // No prediction (the default value)
			return;
		if (!((o = params->find("Colors")) && o->to_number(colors)))
			colors = 1;
		if (!((o = params->find("BitsPerComponent")) && o->to_number(bpc)))
			bpc = 8;
		if (!((o = params->find("Columns")) && o->to_number(cols)))
			cols = 1;

		switch (predictor_id) {
			case 10: case 11: case 12: case 13: case 14: case 15:
				m_predictor = new PredictorPNG(cols, colors, bpc);
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

