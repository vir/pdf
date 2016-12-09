#include "Content.hpp"

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

