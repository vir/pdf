
#ifndef PDF_EXCEPTIONS_HPP
#define PDF_EXCEPTIONS_HPP

//#include <fstream>
#include <string>
#include <sstream>

namespace PDF {

/// PDF File format exception class
class FormatException:public std::exception
{
  private:
    std::string s;
    long off;
  public:
    /**
     * Constructs exception object
     * \param descr problem description.
     * \param offset offset from the beginning of file at (or near) which
     * error encountered.
     */
    FormatException(std::string descr="", long offset=-1) throw()
      :s(descr),off(offset)
    {
    }
    virtual ~FormatException() throw() {}
    virtual const char * what() throw()
    {
      std::stringstream r;
      r << "PDF format error: " << s;
      if(off>=0) r << " at offset " << off;
      return r.str().c_str();
    }
};


}; // namespace PDF


#endif /* PDF_EXCEPTIONS_HPP */

