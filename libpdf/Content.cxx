#include "Content.hpp"
#include "Object.hpp"
#include "ObjStrm.hpp"
#include "Media.hpp"

PDF::Content::~Content()
{
	// delete all operators
	for(unsigned int i = 0; i<operators.size(); i++) { delete operators[i]; }
}

/** \brief Parses page data to sequence of operators, stored in Page object
*/
bool PDF::Content::parse(const std::vector<char> & data)
{
	std::stringstream ss(std::string(data.begin(), data.end()));
	ObjIStream strm(ss);
	strm.throw_eof(false);
	strm.allow_keywords(true);

	std::vector<Object *> * args = NULL;
	Object * o = NULL;
	while((o = strm.read_direct_object()))
	{
		Keyword * kw = dynamic_cast<Keyword *>(o);
		if(kw)
		{
			std::streamoff offs = kw->m_offset;
			if(args)
				offs = (*args->begin())->m_offset;
			operators.push_back(new Operator(offs, kw->value(), args));
			args = NULL;
			delete kw; // name already copied
		}
		else
		{
			if(!args) args = new std::vector<Object *>;
			args->push_back(o);
		}
	}
	assert(!args);

	// delete tail of (broken?) content stream
	if(args)
	{
		for(std::vector<Object *>::iterator it = args->begin(); it != args->end(); it++) delete *it;
		delete args;
	}

	return true;
}

std::streamoff PDF::Content::get_operator_offset(unsigned int n) const
{
	if(n < operators.size()) {
		Operator* o = operators.at(n);
		return o->offset();
	}
	return 0;
}

void PDF::Content::draw(PDF::Content::ResourceProvider& res, Media& m, unsigned int operators_num)
{
	Render r(gs, res, m);
	for(unsigned int operator_index = 0; operator_index < operators_num; operator_index++)
	{
		Operator * op = operators[operator_index];
		if(1) {
			std::stringstream ss;
			ss << operator_index << ": " << op->dump();
			m.Debug(operator_index, ss.str(), gs);
		}
		if(!r.draw(*op))
			m.Debug(operator_index, std::string("Ignoring operator ") + op->dump() + " in " + r.mode_string() + " mode", gs);
	}
	/* Output some debugging information */
	bool dump_debug_info = operators_num < operators.size();
	if(dump_debug_info) {
		std::ostringstream ss;
		ss << "=== Page drawing finished (" << operators_num << " operators executed) ===" << std::endl;
		gs.dump(ss);
		r.dump(ss);
		//if(operators_num < operators.size())
			ss << "Next operator: " << operators[operators_num]->dump() << std::endl;
		m.Debug(operators_num, ss.str(), gs);
	}
}

PDF::Content::Operator::Operator(std::streamoff offset, std::string op, std::vector<Object*>* a)
	:m_offset(offset), m_name(op), m_args(a)
{
}

PDF::Content::Operator::~Operator()
{
	if(!m_args) return;
	for(std::vector<Object *>::iterator it = m_args->begin(); it != m_args->end(); it++) delete *it;
	delete m_args;
}

