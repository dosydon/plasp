#include <pddlparse/detail/parsing/PredicateDeclaration.h>

#include <pddlparse/ParserException.h>
#include <pddlparse/detail/ASTContext.h>
#include <pddlparse/detail/parsing/VariableDeclaration.h>

namespace pddl
{
namespace detail
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PredicateDeclaration
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void parseAndAddPredicateDeclaration(Context &context, ast::Domain &domain)
{
	auto &tokenizer = context.tokenizer;
	tokenizer.expect<std::string>("(");

	auto name = tokenizer.getIdentifier();

	tokenizer.skipWhiteSpace();

	// Parse parameters
	auto parameters = parseVariableDeclarations(context, domain);

	tokenizer.expect<std::string>(")");

	domain.predicates.emplace_back(std::make_unique<ast::PredicateDeclaration>(std::move(name), std::move(parameters)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void parseAndAddPredicateDeclarations(Context &context, ast::Domain &domain)
{
	auto &tokenizer = context.tokenizer;
	tokenizer.skipWhiteSpace();

	while (tokenizer.currentCharacter() != ')')
	{
		parseAndAddPredicateDeclaration(context, domain);

		tokenizer.skipWhiteSpace();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}