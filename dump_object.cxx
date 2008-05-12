/*
 * Dump single object from a given stream object
 * 
 */
#include "File.hpp"
#include "Object.hpp"
#include "Document.hpp"
#include "OH.hpp"
#include <iostream>

using namespace std;

void help(const char * myname);
long get_page_descriptor_object(PDF::File & pf, long pagenum);
void dump_generic_object(PDF::File & pf, long objnum);
void dump_page_content(PDF::File & pf, long pagenum);

int main(int argc, char * argv[])
{
	const char * fname;
	int objnum;

  if(argc != 3) {
		help(argv[0]);
    return -1;
  }

	fname = argv[1];

  try {
    clog << "Constructing File" << endl;
    PDF::File pf(fname);
//		pf.debug(3);
    if(!pf.load())
    {
      cerr << "Can't load " << fname << endl;
      pf.close();
      return 10;
    }

		switch(*argv[2]) {
			case 'p':
				objnum = atol(argv[2]+1);
				clog << "Finding page " << objnum << "'s object" << endl;
				objnum = get_page_descriptor_object(pf, objnum);
				dump_generic_object(pf, objnum);
				break;
			case 'c':
				dump_page_content(pf, atol(argv[2]+1));
				break;
			default:
				objnum = atol(argv[2]);
				dump_generic_object(pf, objnum);
				break;
		}
  }
  catch(...)
  {
    cerr << "Exception!!" << endl;
  }
  return 0;
}

void help(const char * myname)
{
	cout << "Usage:" << endl;
	cout << "\tDump object:\t" << myname << " file.pdf <object_number>" << endl;
	cout << "\tDump page descriptor:\t" << myname << " file.pdf p<object_number>" << endl;
	cout << "\tDump page content:\t" << myname << " file.pdf c<object_number>" << endl;
}

long get_page_descriptor_object(PDF::File & pf, long pagenum)
{
	PDF::Document doc(pf, 5);
	PDF::ObjId id = doc.get_page_objid(pagenum-1);
	return id.num;
}

void dump_generic_object(PDF::File & pf, long objnum)
{
	PDF::ObjId id = { objnum, 0 };
	clog << "Object id is " << id.dump() << endl;

	clog << "Loading object from file" << endl;
	PDF::Object * o=pf.load_object(id);
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

void dump_page_content(PDF::File & pf, long pagenum)
{
	PDF::Document doc(pf, 5);
	PDF::OH pagenode = doc.get_page_node(pagenum - 1);
	PDF::OH contents_h = pagenode.find("Contents");
	contents_h.expand();
	if(!contents_h) {
		std::clog << " Page " << pagenum << ": no contents" << std::endl;
		exit(1);
	} else {
		std::vector<char> data;
		PDF::Stream * stream;
		if((stream=dynamic_cast<PDF::Stream *>(contents_h.obj()))) {
//			stream->get_data(data);
			cout << stream->value();
		} else {
			contents_h.cast<PDF::Array *>("Page content is not a Stream and not an Array. That is wrong. I give up.");
			for(unsigned int i=0; i<contents_h.size(); i++) {
				PDF::OH s = contents_h[i];
				s.expand();
				stream=s.cast<PDF::Stream *>("Page content is not a stream?!?!!?");
//				stream->get_data(data);
				cout << stream->value();
			}
		}
	}
}

