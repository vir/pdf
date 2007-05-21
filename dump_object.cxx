/*
 * Dump single object from a given stream object
 * 
 */
#include "File.hpp"
#include "Object.hpp"
#if 0
#include "Document.hpp"
#include "OH.hpp"
#endif
#include <iostream>

using namespace std;

int main(int argc, char * argv[])
{
  if(argc != 3)
  {
    cout << "Usage:" << endl << "\t" << argv[0] << " file.pdf object_number" << endl;
    return -1;
  }

  try {
    clog << "Constructing File" << endl;
    PDF::File pf(argv[1]);
		pf.debug(3);
    if(!pf.load())
    {
      cerr << "Can't load " << argv[1] << endl;
      pf.close();
      return 10;
    }

    PDF::ObjId id;
    id.num=atol(argv[2]);
    id.gen=0;
		clog << "Object id is " << id.dump() << endl;

#if 0
    clog << "Constructing Document" << endl;
    PDF::Document pd(pf);

    clog << "Fetching object from document" << endl;
    PDF::OH handle=pd.get_object(id);
    PDF::Object * o=handle.obj();
#else
    clog << "Loading object from file" << endl;
    PDF::Object * o=pf.load_object(id);
#endif
    cout << "Object type: " << o->type() << endl;
    cout << o->dump() << endl;

    PDF::Stream * stream=dynamic_cast<PDF::Stream *>(o);
    if(stream)
    {
			string v = stream->value();
      clog << "Dumping Stream object (" << v.length() << " bytes)" << endl;
			cout << v;
    }
  }
  catch(...)
  {
    cerr << "Exception!!" << endl;
  }
  return 0;
}


