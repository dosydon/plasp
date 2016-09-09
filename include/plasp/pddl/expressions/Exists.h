#ifndef __PLASP__PDDL__EXPRESSIONS__EXISTS_H
#define __PLASP__PDDL__EXPRESSIONS__EXISTS_H

#include <plasp/pddl/expressions/Quantified.h>

namespace plasp
{
namespace pddl
{
namespace expressions
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Exists
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Exists: public QuantifiedCRTP<Exists>
{
	public:
		static const Expression::Type ExpressionType = Expression::Type::Exists;

		static const std::string Identifier;

	public:
		ExpressionPointer decomposed(DerivedPredicates &derivedPredicates) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
