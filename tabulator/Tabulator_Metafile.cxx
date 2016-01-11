#include "Tabulator.hpp"

void Tabulator::Metafile::Size(PDF::Point size)
{
	if(rotation % 2) {
		myarea = PDF::Rect(35, 0, size.y, size.x);
		page_height = size.x;
	} else {
		myarea = PDF::Rect(0, 0, size.x, size.y);
		page_height = size.y;
	}
	matrix.set_unity();
	switch(rotation) {
		case 0: break;
		case 1: matrix.rotate( 90.0); matrix.offset(size.y, 0); break;
		case 2: matrix.rotate(180.0); matrix.offset(size.x, size.y); break;
		case 3: matrix.rotate(270.0); matrix.offset(0, size.x); break;
		default: break;
	}
}


void Tabulator::Metafile::Text(PDF::Rect pos, double angle, std::wstring text, bool visible, const PDF::GraphicsState& gs)
{
	if(!myarea.in(pos.offset())) {
		std::wclog << L"Skipping text <<" << text << L">> - not in my area" << std::endl;
		return;
	}
//	std::clog << "Text" << pos.dump() << std::endl;
	all_text.insert(TextBlock(PDF::Point(pos.x1, page_height - pos.y1), angle, text, pos.width(), pos.height()));
}

/**
 * Sort lines to horizontal/vertical/short, store them in h_lines and v_lines
 * and make sure that p1.[xy] < p2.[xy]
 */
void Tabulator::Metafile::Line(const PDF::Point & p1, const PDF::Point & p2, const PDF::GraphicsState& gs)
{
	if(!myarea.in(p1) || !myarea.in(p2)) {
		std::clog << "Skipping line " << p1.dump() << "-" << p2.dump() << ">> - not in my area" << std::endl;
		return;
	}

	Coord x1(p1.x);
	Coord y1(page_height - p1.y);
	Coord x2(p2.x);
	Coord y2(page_height - p2.y);

	if(x1 == x2 && y1 != y2) { // vertical line
//		std::clog << "V-Line" << p1.dump() << "-" << p2.dump() << std::endl;
		std::pair<Coord, Coord> y(y1, y2);
		if(y2 < y1) std::swap(y.first, y.second);
		v_lines.insert(std::pair<Coord, std::pair<Coord, Coord> >(x1, y));
		return;
	}
	if(x1 != x2 && y1 == y2) { // horizontal line
//		std::clog << "H-Line" << p1.dump() << "-" << p2.dump() << std::endl;
		std::pair<Coord, Coord> x(x1, x2);
		if(x2 < x1) std::swap(x.first, x.second);
		h_lines.insert(std::pair<Coord, std::pair<Coord, Coord> >(y1, x));
		return;
	}

//	std::clog << "Skipping invalid line " << p1.dump() << "-" << p2.dump() << std::endl;
}

