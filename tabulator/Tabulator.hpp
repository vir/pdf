#ifndef TABULATOR_HPP_INCLUDED
#define TABULATOR_HPP_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <vector>
#include <list>
#include <libpdf/PDF.hpp>

class Tabulator
{
	public:
		/** Coordinate which compares with a specified tolerance */
		class Coord {
			private:
				double v;
			public:
				Coord(double d=0):v(d) {}
				bool operator < (const Coord & c) const { return compare(v, c.v) < 0; }
				bool operator == (const Coord & c) const { return compare(v, c.v) == 0; }
				bool operator != (const Coord & c) const { return compare(v, c.v) != 0; }
				operator double() const { return v; }
				inline static int compare(double d1, double d2, double delta=3)
				{
					if(d1+delta < d2) return -1;
					if(d1-delta > d2) return +1;
					return 0;
				}
		};
		/** Abstract surface on which PDF page will be drawn */
		class Metafile:public PDF::Media
		{
			private:
				PDF::CTM matrix;
				int rotation;
				PDF::Rect myarea;
			public:
				typedef	std::multimap<Coord, std::pair<Coord, Coord> > LineMap;
				typedef std::map<PDF::Point, std::wstring> TextMap;

				Metafile():rotation(0),myarea(0,0,10000,10000) {}
				LineMap h_lines, v_lines;
				TextMap all_text;
				virtual void Text(PDF::Point pos, std::wstring text);
				virtual void Line(const PDF::Point & p1, const PDF::Point & p2);
				virtual const PDF::CTM & Matrix() { return matrix; }
				virtual void Size(PDF::Point size);
				void Clear() {
					h_lines.clear();
					v_lines.clear();
					all_text.clear();
				}
		};
		/** Grid, dividing table into cells */
		class Grid
		{
			public:
				class Line:public std::vector<bool>
				{
				};
				typedef std::map< Coord, Line > KnotsMap;
				typedef KnotsMap::const_iterator KnotsIterator;
				KnotsMap h_knots, v_knots;
				void build(const Metafile * mf);
				unsigned int find_col(double x) const;
				unsigned int find_row(double y) const;
				std::string dump() const;
		};
		/** Table of cells */
		class Table
		{
			public: /* types */
				/** Table cell */
				class Cell
				{
					private:
						std::map<PDF::Point, std::wstring> text;
					public: // data
						bool is_header;
						unsigned int colspan, rowspan;
					public: // methods
						Cell():is_header(false) {}
						void addtext(const PDF::Point & p, const std::wstring & s) { text[p]=s; }
						std::wstring celltext() const;
				};
				/** Table exporter interface */
				class Exporter
				{
					public:
						virtual void table_begin(unsigned int ncols, unsigned int nrows) {}
						virtual void row_begin(unsigned int r) {}
						virtual void cell(std::wstring text, unsigned int c, unsigned int r) {};
						virtual void cell(const Cell * cptr, unsigned int c, unsigned int r) { cell(cptr?cptr->celltext():L"", c, r); }
						virtual void row_end() {}
						virtual void table_end() {}
				};
			private:
				std::list<Cell> all_cells;
				std::vector< std::vector<Cell *> > cells;
			public: /* methods */
				Grid grid;
				void resize(unsigned int cols, unsigned int rows);
				Cell * cell(unsigned int col, unsigned int row, bool create=true);
				const Cell * cell(unsigned int col, unsigned int rowi) const;
				void eat_text(const Tabulator::Metafile * mf);
				void output(Exporter * ex) const;
				void clear();
				std::string dump() const;
		};
	public:
		Metafile metafile;
		Grid grid;
		Table table;
		struct Options {
			bool find_more_rows;
			unsigned int find_rows_column;
			Options():find_more_rows(false),find_rows_column(0) { }
		} options;
		void set_area(PDF::Rect r);
		void set_rotation(int rot);
		void set_tolerance(double tx, double ty);
		void load_page(PDF::Page * page);
		void build_grid();
		void fill_table_with_text();
		void full_process(PDF::Page * page);
		void dump() const;
		void output(Table::Exporter * ex) const { table.output(ex); }

    Tabulator() {  }
    virtual ~Tabulator()   {  };
};


#endif /* TABULATOR_HPP_INCLUDED */

