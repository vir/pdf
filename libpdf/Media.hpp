
#ifndef PDF_MEDIA_HPP
#define PDF_MEDIA_HPP

#include <string>

namespace PDF {

/// Pure virtual class, defines interface to draw on some media (i.e. paper).
class Media
{
  public:
    virtual ~Media() {};
    virtual Point Size(Point unity)=0;
    virtual void Text(Point pos, const Font * font, std::wstring text)=0;
    virtual void Line(const Point & p1, const Point & p2)=0;
};
  
};

#endif /* PDF_MEDIA_HPP */



