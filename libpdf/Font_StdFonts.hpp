#ifndef STDFONT_METRICS_H_INCLUDED
#define STDFONT_METRICS_H_INCLUDED
#include <map>
#include <string>

void load_stdfont_widths_table(std::map<int, unsigned long> & glyphwidths, std::map<wchar_t, unsigned long> & unicodewidths, std::string font);

#endif /* STDFONT_METRICS_H_INCLUDED */

