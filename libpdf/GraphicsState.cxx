#include "GraphicsState.hpp"

PDF::GraphicsStateStack::GraphicsStateStack()
{
	gs = new GraphicsState();
}

PDF::GraphicsStateStack::~GraphicsStateStack()
{
	clear();
}

void PDF::GraphicsStateStack::push()
{
	gstack.push(gs);
	gs = new GraphicsState(*gs);
}

void PDF::GraphicsStateStack::pop()
{
	if(gs) delete gs;
	if(gstack.empty())
		throw WrongPageException("GraphicsState stack underrun");
	gs = gstack.top();
	gstack.pop();
}

void PDF::GraphicsStateStack::dump(std::ostream & ss)
{
	if(gs) {
		ss << "Graphics state:";
		gs->dump(ss);
		ss << std::endl;
	}
	ss << "Graphics state stack: " << gstack.size() << " entries" << std::endl;
}

void PDF::GraphicsStateStack::clear()
{
	// delete all forgotten graphics stack contents
	while(!gstack.empty()) { delete gstack.top(); gstack.pop(); }
	if(gs) delete gs;
}

void PDF::GraphicsStateStack::inherit(const GraphicsStateStack & parent)
{
	clear();
	gs = new GraphicsState(*parent.gs);
}

