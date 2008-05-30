#ifndef TABULATOR_HPP_INCLUDED
#define TABULATOR_HPP_INCLUDED

#include <string>
#include <vector>
#include <map>
//#include <iostream>

//#include "PDF.hpp"
#include "libpdf/Media.hpp"
//#include "utf8.hpp"

class Tabulator:public PDF::Media
{
	public:
		class Coord;
		class Cell;
  private:
		PDF::CTM matrix;
    double pheight;
		int rotation;
		PDF::Rect myarea;
  public:
    typedef PDF::Point Point;
    class PLine
    {
      public:
        Point p1, p2;
        PLine(Point z1, Point z2):p1(z1),p2(z2) {}
    };
    class TextLines:public std::map<PDF::Point, std::string>
		{
		};
    TextLines all_text;
		std::vector<PLine> h_lines, v_lines;
		bool output_csv;

    Tabulator(int rot = 0):pheight(0),rotation(rot),myarea(0,0,10000,10000),output_csv(true) {  }
    virtual ~Tabulator()   {  };
    virtual void Text(Point pos, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
    void dump() const;
    void chew(bool skip_headers=false);
		virtual const PDF::CTM & Matrix() { return matrix; }
    virtual void Size(Point size);
};

#if 0
class Tabulator::Coord
{
  private:
    double v;
  public:
    Coord(double d):v(d) {}
    bool operator < (const Coord & c) const { return compare(v, c.v) < 0; }
    bool operator == (const Coord & c) const { return compare(v, c.v) == 0; }
    operator double() const { return v; }
};

class Tabulator::Cell
{
	private:
		map<PDF::Point, string> text;
	public:
		bool is_header;
		Cell():is_header(false) {}
		void addtext(const PDF::Point & p, const string & s) { text[p]=s; }
		string celltext()
		{
      string r;
			map<PDF::Point, string>::const_iterator tpit;
      for(tpit=text.begin(); tpit!=text.end(); tpit++)
      {
        /// remove hyphenation XXX \todo mess with unicode hypheation character (if any?)
        if(!r.empty() && r[r.size()-1]=='-') r.resize(r.size()-1);
        r+=tpit->second;
      }
			return r;
		}
		string html()
		{
			if(!is_header) {
				return string("<td>")+celltext()+"</td>";
			} else {
				return string("<th>")+celltext()+"</th>";
			}
		}
};
#endif

#endif /* TABULATOR_HPP_INCLUDED */

