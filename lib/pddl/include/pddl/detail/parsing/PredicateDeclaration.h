#ifndef __PDDL__DETAIL__PARSING__PREDICATE_DECLARATION_H
#define __PDDL__DETAIL__PARSING__PREDICATE_DECLARATION_H

#include <pddl/ASTForward.h>
#include <pddl/Context.h>

namespace pddl
{
namespace detail
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PredicateDeclaration
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void parseAndAddPredicateDeclarations(Context &context, ast::Domain &domain);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
