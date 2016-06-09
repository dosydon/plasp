#include <plasp/pddl/expressions/Either.h>

#include <plasp/pddl/ExpressionVisitor.h>

namespace plasp
{
namespace pddl
{
namespace expressions
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Either
//
////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string Either::Identifier = "either";

////////////////////////////////////////////////////////////////////////////////////////////////////

void Either::accept(plasp::pddl::ExpressionVisitor &expressionVisitor) const
{
	expressionVisitor.visit(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
