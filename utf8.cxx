
#include <string>

std::string ws2utf8(const std::wstring & ws)
{
  std::string s;
  for(unsigned int i=0; i<ws.length(); i++)
  {
    unsigned long n=ws[i];

    if(n & ~0x7F)
    {
      int x=0;
      unsigned char cbuf[6];
      unsigned char header=0x80;
      
      while(n & ~0x3F)
      {
        cbuf[x++]=n&0x3F; n>>=6;
        header>>=1; header|=0x80;
      }

      s.push_back(n | header); x--;
      while(x>=0) { s.push_back(cbuf[x--]|0x80); }
    }
    else { s.push_back(n); }
  }
  return s;
}



