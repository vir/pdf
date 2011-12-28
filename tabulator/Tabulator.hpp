#ifndef TABULATOR_HPP_INCLUDED
#define TABULATOR_HPP_INCLUDED

// get rid of "identifier was truncated to '255' characters..."
#ifdef _MSC_VER
# pragma warning(disable : 4786)
#endif

#include <string>
#include <vector>
#include <map>
#include <set>
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
				Coord operator - (const Coord & c) const { return v - c.v; }
				Coord operator + (const Coord & c) const { return v + c.v; }
				Coord operator - (double d) const { return v - d; }
				Coord operator + (double d) const { return v + d; }
				Coord operator / (double d) const { return v / d; }
				Coord operator * (double d) const { return v * d; }
				operator double() const { return v; }
				inline static int compare(double d1, double d2, double delta=3)
				{
					if(d1+delta < d2) return -1;
					if(d1-delta > d2) return +1;
					return 0;
				}
		};
		class TextBlock
		{
			public:
				static bool debug;
				PDF::Point pos;
				std::wstring text;
				double width;
				double height;
				double angle;
				PDF::Rect bounds() const;
				bool merge_ok(const TextBlock & oth) const;
				TextBlock operator +(const TextBlock & oth) const;
				bool operator <  (const TextBlock & oth) const { int r = Coord::compare(pos.y, oth.pos.y); return r?(r < 0):(pos.x < oth.pos.x); }
				//bool operator == (const TextBlock & oth) const { return pos < oth.pos; }
				bool operator == (const TextBlock & oth) const { return Coord::compare(pos.y, oth.pos.y) == 0 && Coord::compare(pos.x, oth.pos.x) == 0 && text == oth.text; }
				TextBlock & operator = (const TextBlock & oth) { pos = oth.pos; text = oth.text; width = oth.width; height = oth.height; angle = oth.angle; return *this; }
				TextBlock();
				TextBlock(const PDF::Point & p, std::wstring s);
				TextBlock(const PDF::Point & p, double a, std::wstring s, double w, double h);
				std::string dump() const;
		};
		/** Abstract surface on which PDF page will be drawn */
		class Metafile:public PDF::Media
		{
			private:
				PDF::CTM matrix;
				int rotation;
				PDF::Rect myarea;
				double page_height;
			public:
				typedef	std::multimap<Coord, std::pair<Coord, Coord> > LineMap;
				typedef std::set<TextBlock> TextMap;

				Metafile():rotation(0),myarea(0,0,10000,10000) {}
				LineMap h_lines, v_lines;
				TextMap all_text;
//				virtual void Text(PDF::Point pos, std::wstring text);
				virtual void Text(PDF::Point pos, double angle, std::wstring text, double twidth, double theight);
				virtual void Line(const PDF::Point & p1, const PDF::Point & p2);
				virtual const PDF::CTM & Matrix() { return matrix; }
				virtual void Size(PDF::Point size);
				void Clear() {
					h_lines.clear();
					v_lines.clear();
					all_text.clear();
				}
				void set_area(PDF::Rect r) { myarea = r; }
				void set_rotation(int rot) { rotation = rot; }
		};
		/** Grid, dividing table into cells */
		class Grid
		{
			public:
				class Line:public std::vector<bool>
				{
					public:
						Line() {}
						Line(unsigned int size, bool defval = false):std::vector<bool>(size, defval) {}
						void set_bits(unsigned int b1, unsigned int b2, bool value = true)
						{
							for(unsigned int i = b1; i <= b2; i++) { at(i) = value; }
						}
						std::string dump() const;
				};
				struct {
					double bottom;
				} margins;
				typedef std::map< Coord, Line > KnotsMap;
				typedef KnotsMap::const_iterator KnotsIterator;
				KnotsMap h_knots, v_knots;
				double headers_end; /**< y coord of line, dividing headers and body */
				Grid():headers_end(-1E10) {}
				void build(const Metafile * mf);
				int find_col(double x) const;
				int find_row(double y) const;
				std::string dump() const;
				void clear() {
					h_knots.clear();
					v_knots.clear();
					headers_end = -1E10;
				}
				void split_column(unsigned int col, double x);
				bool auto_split_column(unsigned int col, const Tabulator::Metafile * mf);
				void add_horizontal_line(double y)
				{
					KnotsMap::iterator iit = v_knots.insert(KnotsMap::value_type(y, Line(h_knots.size()-1, true))).first;
				}

		};
		/** Table of cells */
		class Table
		{
			public: /* types */
				/** Table cell */
				class Cell
				{
					private:
						std::set<TextBlock> text;
					public: // data
						bool is_header;
						unsigned int colspan, rowspan;
					public: // methods
						Cell():is_header(false),colspan(1),rowspan(1) {}
						unsigned int mergeblocks();
						void addtext(const TextBlock & b) { text.insert(b); }
						std::wstring celltext() const;
						void set_spans(unsigned int cs, unsigned int rs) { colspan = cs; rowspan = rs; }
				};
				/** Table exporter interface */
				class Exporter
				{
					public:
						virtual ~Exporter() {}
						virtual bool set_params(std::string pstr) { return true; }
						virtual void page_begin(std::string fname, unsigned int pnum) {}
						virtual void table_begin(unsigned int ncols, unsigned int nrows) {}
						virtual void row_begin(unsigned int r) {}
						virtual void cell(std::wstring text, unsigned int c, unsigned int r) {};
						virtual void cell(const Cell * cptr, unsigned int c, unsigned int r) { cell(cptr?cptr->celltext():L"", c, r); }
						virtual void cell(const Cell * cptr, bool hidden, unsigned int c, unsigned int r, unsigned int cs, unsigned int rs) { cell(hidden?NULL:cptr, c, r); }
						virtual void row_end() {}
						virtual void table_end() {}
						virtual void page_end() {}
				};
				class CellPtr
				{
					private:
						Cell * m_ptr;
						bool m_hidden;
					public:
						explicit CellPtr(Cell * c = NULL):m_ptr(c),m_hidden(false) {}
						CellPtr & operator=(Cell * c) { m_ptr = c; return *this; }
						Cell & operator*() { return *m_ptr; }
						Cell * operator->() { return m_ptr; }
						Cell * get() { return m_ptr; }
						void hide() { m_hidden = true; }
						void unhide() { m_hidden = false; }
						bool hidden() const { return m_hidden; }
						const Cell * get() const { return m_ptr; }
				};
			private:
				std::list<Cell> all_cells;
				std::vector< std::vector<CellPtr> > cells;
			public: /* methods */
				unsigned int ncols() const;
				unsigned int nrows() const;
				Grid grid;
				void resize(unsigned int cols, unsigned int rows);
				Cell * cell(unsigned int col, unsigned int row, bool create=true);
				void span(unsigned int col, unsigned int row, unsigned int cs, unsigned int rs);
				bool is_hidden(unsigned int col, unsigned int row) { return cells[row][col].hidden(); }
				const Cell * cell(unsigned int col, unsigned int rowi) const;
				void postprocess();
				void output(Exporter * ex) const;
				void clear();
				std::string dump() const;
		};
		struct Options {
			bool find_table_header;
			unsigned int find_more_rows_column;
			bool postprocess;
			bool find_joined_cells;
			std::vector<int> split_columns;
			Options():find_table_header(true),find_more_rows_column(0),postprocess(false),find_joined_cells(true) { }
		};
	public:
		Metafile metafile;
		Grid grid;
		Table table;
		Options options;
		void set_tolerance(double tx, double ty);
		void flush() { metafile.Clear(); grid.clear(); table.clear(); }
		void load_page(PDF::Page * page);
		void build_grid();
		void prepare_table();
		void fill_table_with_text();
		void full_process(PDF::Page * page);
		void dump() const;
		void output(Table::Exporter * ex) const { table.output(ex); }

    Tabulator() {  }
    virtual ~Tabulator()   {  };
	static bool debug;
};


#endif /* TABULATOR_HPP_INCLUDED */

