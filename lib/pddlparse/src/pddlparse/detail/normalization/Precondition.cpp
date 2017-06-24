#include <pddlparse/detail/normalization/Precondition.h>

#include <pddlparse/AST.h>
#include <pddlparse/Exception.h>
#include <pddlparse/NormalizedAST.h>
#include <pddlparse/detail/normalization/AtomicFormula.h>
#include <pddlparse/detail/normalization/CollectFreeVariables.h>
#include <pddlparse/detail/normalization/Reduction.h>

namespace pddl
{
namespace detail
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Precondition
//
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::AndPointer<ast::Precondition> &and_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates);
normalizedAST::Literal normalizeNestedImpl(ast::ExistsPointer<ast::Precondition> &exists, normalizedAST::DerivedPredicateDeclarations &derivedPredicates);
normalizedAST::Literal normalizeNestedImpl(ast::ForAllPointer<ast::Precondition> &forAll, normalizedAST::DerivedPredicateDeclarations &derivedPredicates);
normalizedAST::Literal normalizeNestedImpl(ast::ImplyPointer<ast::Precondition> &, normalizedAST::DerivedPredicateDeclarations &);
normalizedAST::Literal normalizeNestedImpl(ast::NotPointer<ast::Precondition> &not_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates);
normalizedAST::Literal normalizeNestedImpl(ast::OrPointer<ast::Precondition> &or_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates);
normalizedAST::Literal normalizeNestedImpl(ast::AtomicFormula &, normalizedAST::DerivedPredicateDeclarations &);

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::DerivedPredicatePointer addDerivedPredicate(const std::vector<normalizedAST::VariableDeclaration *> &parameters, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	auto name = "derived-predicate-" + std::to_string(derivedPredicates.size() + 1);

	normalizedAST::DerivedPredicate::Arguments arguments;
	arguments.reserve(parameters.size());

	for (const auto &parameter : parameters)
		arguments.emplace_back(std::make_unique<normalizedAST::Variable>(parameter));

	derivedPredicates.emplace_back(std::make_unique<normalizedAST::DerivedPredicateDeclaration>());

	auto *derivedPredicate = derivedPredicates.back().get();
	derivedPredicate->name = std::move(name);
	derivedPredicate->parameters = std::move(parameters);

	return std::make_unique<normalizedAST::DerivedPredicate>(std::move(arguments), derivedPredicate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::AndPointer<ast::Precondition> &and_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	std::vector<normalizedAST::VariableDeclaration *> parameters;
	VariableStack variableStack;

	collectFreeVariables(and_, parameters, variableStack);

	auto derivedPredicate = addDerivedPredicate(parameters, derivedPredicates);

	normalizedAST::And<normalizedAST::Literal>::Arguments normalizedArguments;
	normalizedArguments.reserve(and_->arguments.size());

	for (auto &argument : and_->arguments)
		normalizedArguments.emplace_back(argument.match(
			[&](auto &x) -> normalizedAST::Literal
			{
				return normalizeNestedImpl(x, derivedPredicates);
			}));

	derivedPredicate->declaration->precondition = std::make_unique<normalizedAST::And<normalizedAST::Literal>>(std::move(normalizedArguments));

	// TODO: investigate, could be a compiler bug
	return std::move(derivedPredicate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::ExistsPointer<ast::Precondition> &exists, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	std::vector<normalizedAST::VariableDeclaration *> parameters;
	VariableStack variableStack;

	collectFreeVariables(exists, parameters, variableStack);

	auto derivedPredicate = addDerivedPredicate(parameters, derivedPredicates);

	derivedPredicate->declaration->existentialParameters = std::move(exists->parameters);
	derivedPredicate->declaration->precondition = exists->argument.match([&](auto &x){return normalizeNestedImpl(x, derivedPredicates);});

	// TODO: investigate, could be a compiler bug
	return std::move(derivedPredicate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::ForAllPointer<ast::Precondition> &, normalizedAST::DerivedPredicateDeclarations &)
{
	// “forall” expressions should be reduced to negated “exists” statements at this point
	throw std::logic_error("precondition not in normal form (forall), please report to the bug tracker");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::ImplyPointer<ast::Precondition> &, normalizedAST::DerivedPredicateDeclarations &)
{
	// “imply” expressions should be reduced to disjunctions at this point
	throw std::logic_error("precondition not in normal form (imply), please report to the bug tracker");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::NotPointer<ast::Precondition> &not_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	// “not” expressions may be nested one time to form simple literals
	if (not_->argument.is<ast::AtomicFormula>())
		return std::make_unique<normalizedAST::Not<normalizedAST::AtomicFormula>>(normalize(std::move(not_->argument.get<ast::AtomicFormula>())));

	std::vector<normalizedAST::VariableDeclaration *> parameters;
	VariableStack variableStack;

	collectFreeVariables(not_, parameters, variableStack);

	auto derivedPredicate = addDerivedPredicate(parameters, derivedPredicates);

	auto normalizedArgumentLiteral = not_->argument.match(
		[&](auto &x) -> normalizedAST::Literal
		{
			return normalizeNestedImpl(x, derivedPredicates);
		});

	// Multiple negations should be eliminated at this point
	if (normalizedArgumentLiteral.is<normalizedAST::NotPointer<normalizedAST::AtomicFormula>>())
		throw std::logic_error("precondition not in normal form (multiple negation), please report to the bug tracker");

	auto &normalizedArgument = normalizedArgumentLiteral.get<normalizedAST::AtomicFormula>();

	derivedPredicate->declaration->precondition = std::make_unique<normalizedAST::Not<normalizedAST::AtomicFormula>>(std::move(normalizedArgument));

	// TODO: investigate, could be a compiler bug
	return std::move(derivedPredicate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::OrPointer<ast::Precondition> &or_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	std::vector<normalizedAST::VariableDeclaration *> parameters;
	VariableStack variableStack;

	collectFreeVariables(or_, parameters, variableStack);

	auto derivedPredicate = addDerivedPredicate(parameters, derivedPredicates);

	normalizedAST::Or<normalizedAST::Literal>::Arguments normalizedArguments;
	normalizedArguments.reserve(or_->arguments.size());

	for (auto &argument : or_->arguments)
		normalizedArguments.emplace_back(argument.match(
			[&](auto &x) -> normalizedAST::Literal
			{
				return normalizeNestedImpl(x, derivedPredicates);
			}));

	derivedPredicate->declaration->precondition = std::make_unique<normalizedAST::Or<normalizedAST::Literal>>(std::move(normalizedArguments));

	// TODO: investigate, could be a compiler bug
	return std::move(derivedPredicate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeNestedImpl(ast::AtomicFormula &atomicFormula, normalizedAST::DerivedPredicateDeclarations &)
{
	return normalize(std::move(atomicFormula));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::AtomicFormula normalizeImpl(ast::AtomicFormula &&atomicFormula, normalizedAST::DerivedPredicateDeclarations &)
{
	return normalize(std::move(atomicFormula));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Normalize top-level negations
normalizedAST::NotPointer<normalizedAST::AtomicFormula> normalizeImpl(ast::NotPointer<ast::Precondition> &&not_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	// “not” expressions may be nested one time to form simple literals
	if (not_->argument.is<ast::AtomicFormula>())
		return std::make_unique<normalizedAST::Not<normalizedAST::AtomicFormula>>(normalize(std::move(not_->argument.get<ast::AtomicFormula>())));

	auto normalizedArgument = not_->argument.match(
		[&](auto &nested) -> normalizedAST::AtomicFormula
		{
			auto normalizedLiteral = normalizeNestedImpl(nested, derivedPredicates);

			// Multiple negations should be eliminated at this point
			if (normalizedLiteral.template is<normalizedAST::NotPointer<normalizedAST::AtomicFormula>>())
				throw std::logic_error("precondition not in normal form (multiple negation), please report to the bug tracker");

			return std::move(normalizedLiteral.template get<normalizedAST::AtomicFormula>());
		});

	return std::make_unique<normalizedAST::Not<normalizedAST::AtomicFormula>>(std::move(normalizedArgument));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Normalize top-level conjunctions
normalizedAST::AndPointer<normalizedAST::Literal> normalizeImpl(ast::AndPointer<ast::Precondition> &&and_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	normalizedAST::And<normalizedAST::Literal>::Arguments arguments;

	arguments.reserve(and_->arguments.size());

	const auto handleAtomicFormula =
		[&](ast::AtomicFormula &atomicFormula) -> normalizedAST::Literal
		{
			return normalize(std::move(atomicFormula));
		};

	const auto handleNot =
		[&](ast::NotPointer<ast::Precondition> &not_) -> normalizedAST::Literal
		{
			return normalizeImpl(std::move(not_), derivedPredicates);
		};

	const auto handleNested =
		[&](auto &nested) -> normalizedAST::Literal
		{
			return normalizeNestedImpl(nested, derivedPredicates);
		};

	for (auto &&argument : and_->arguments)
	{
		auto normalizedArgument = argument.match(handleAtomicFormula, handleNot, handleNested);

		arguments.emplace_back(std::move(normalizedArgument));
	}

	return std::make_unique<normalizedAST::And<normalizedAST::Literal>>(std::move(arguments));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeImpl(ast::ExistsPointer<ast::Precondition> &&exists, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	return normalizeNestedImpl(exists, derivedPredicates);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeImpl(ast::ForAllPointer<ast::Precondition> &&, normalizedAST::DerivedPredicateDeclarations &)
{
	throw std::logic_error("precondition not in normal form (forall), please report to the bug tracker");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeImpl(ast::ImplyPointer<ast::Precondition> &&, normalizedAST::DerivedPredicateDeclarations &)
{
	throw std::logic_error("precondition not in normal form (imply), please report to the bug tracker");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Literal normalizeImpl(ast::OrPointer<ast::Precondition> &&or_, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	return normalizeNestedImpl(or_, derivedPredicates);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

normalizedAST::Precondition normalize(ast::Precondition &&precondition, normalizedAST::DerivedPredicateDeclarations &derivedPredicates)
{
	// Bring precondition to normal form
	reduce(precondition);

	return precondition.match(
		[&](auto &x) -> normalizedAST::Precondition
		{
			return normalizeImpl(std::move(x), derivedPredicates);
		});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
