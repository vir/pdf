
#include <zlib.h>
#include <vector>
//#include <string>

#include "Filter.hpp"

namespace PDF {

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
  
  r=inflateInit(&zs);
  if(r!=Z_OK) return false;

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

  if(r!=Z_STREAM_END) return false;
  
  dst.resize((char*)zs.next_out-&dst[0]); // adjust output object size
  r=inflateEnd(&zs);
  if(r!=Z_OK) return false;
  
  return true;
}


}; // namespace PDF

