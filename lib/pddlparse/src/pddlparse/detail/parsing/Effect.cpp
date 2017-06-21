#include <pddlparse/detail/parsing/Effect.h>

#include <pddlparse/AST.h>
#include <pddlparse/detail/parsing/AtomicFormula.h>
#include <pddlparse/detail/parsing/Expressions.h>
#include <pddlparse/detail/parsing/Precondition.h>
#include <pddlparse/detail/parsing/Unsupported.h>
#include <pddlparse/detail/parsing/Utils.h>

namespace pddl
{
namespace detail
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<ast::Effect> parseEffectBody(Context &context, ASTContext &astContext, VariableStack &variableStack);
std::experimental::optional<ast::ConditionalEffect> parseConditionalEffect(Context &context, ASTContext &astContext, VariableStack &variableStack);
std::experimental::optional<ast::ConditionalEffect> parseConditionalEffectBody(Context &context, ASTContext &astContext, VariableStack &variableStack);

////////////////////////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<ast::Effect> parseEffect(Context &context, ASTContext &astContext, VariableStack &variableStack)
{
	auto &tokenizer = context.tokenizer;

	tokenizer.skipWhiteSpace();

	std::experimental::optional<ast::Effect> effect;

	if ((effect = parseAnd<ast::Effect>(context, astContext, variableStack, parseEffect))
	    || (effect = parseForAll<ast::Effect>(context, astContext, variableStack, parseEffect))
	    || (effect = parseWhen<ast::Precondition, ast::ConditionalEffect>(context, astContext, variableStack, parsePreconditionBody, parseConditionalEffect)))
	{
		return std::move(effect.value());
	}

	return parseEffectBody(context, astContext, variableStack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<ast::Effect> parseEffectBody(Context &context, ASTContext &astContext, VariableStack &variableStack)
{
	auto &tokenizer = context.tokenizer;

	tokenizer.skipWhiteSpace();

	// Test unsupported expressions first
	const auto position = tokenizer.position();

	tokenizer.expect<std::string>("(");
	tokenizer.skipWhiteSpace();

	const auto expressionIdentifierPosition = tokenizer.position();

	if (tokenizer.testIdentifierAndReturn("assign")
		|| tokenizer.testIdentifierAndReturn("scale-up")
		|| tokenizer.testIdentifierAndReturn("scale-down")
		|| tokenizer.testIdentifierAndReturn("increase")
		|| tokenizer.testIdentifierAndReturn("decrease"))
	{
		tokenizer.seek(position);
		return parseUnsupported(context);
	}

	tokenizer.seek(position);

	// Now, test supported expressions
	std::experimental::optional<ast::Effect> effect;

	if ((effect = parseNot<ast::Effect>(context, astContext, variableStack, parseAtomicFormula))
		|| (effect = parseAtomicFormula(context, astContext, variableStack)))
	{
		return std::move(effect.value());
	}

	tokenizer.seek(expressionIdentifierPosition);
	const auto expressionIdentifier = tokenizer.getIdentifier();

	tokenizer.seek(position);
	throw ParserException(tokenizer.location(), "expression type “" + expressionIdentifier + "” unknown or not allowed in effect body");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<ast::ConditionalEffect> parseConditionalEffect(Context &context, ASTContext &astContext, VariableStack &variableStack)
{
	auto &tokenizer = context.tokenizer;

	tokenizer.skipWhiteSpace();

	std::experimental::optional<ast::ConditionalEffect> conditionalEffect;

	if ((conditionalEffect = parseAnd<ast::ConditionalEffect>(context, astContext, variableStack, parseConditionalEffectBody)))
		return std::move(conditionalEffect.value());

	return parseConditionalEffectBody(context, astContext, variableStack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<ast::ConditionalEffect> parseConditionalEffectBody(Context &context, ASTContext &astContext, VariableStack &variableStack)
{
	auto &tokenizer = context.tokenizer;

	tokenizer.skipWhiteSpace();

	// Test unsupported expressions first
	const auto position = tokenizer.position();

	tokenizer.expect<std::string>("(");
	tokenizer.skipWhiteSpace();

	const auto expressionIdentifierPosition = tokenizer.position();

	if (tokenizer.testIdentifierAndReturn("=")
		|| tokenizer.testIdentifierAndReturn("assign")
		|| tokenizer.testIdentifierAndReturn("scale-up")
		|| tokenizer.testIdentifierAndReturn("scale-down")
		|| tokenizer.testIdentifierAndReturn("increase")
		|| tokenizer.testIdentifierAndReturn("decrease"))
	{
		tokenizer.seek(position);
		return parseUnsupported(context);
	}

	tokenizer.seek(position);

	// Now, test supported expressions
	std::experimental::optional<ast::ConditionalEffect> conditionalEffect;

	if ((conditionalEffect = parseNot<ast::ConditionalEffect>(context, astContext, variableStack, parseAtomicFormula))
		|| (conditionalEffect = parseAtomicFormula(context, astContext, variableStack)))
	{
		return std::move(conditionalEffect.value());
	}

	tokenizer.seek(expressionIdentifierPosition);
	const auto expressionIdentifier = tokenizer.getIdentifier();

	tokenizer.seek(position);
	throw ParserException(tokenizer.location(), "expression type “" + expressionIdentifier + "” unknown or not allowed in conditional effect body");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
