#include <plasp/pddl/Description.h>

#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

#include <plasp/input/ParserException.h>
#include <plasp/pddl/IO.h>

namespace plasp
{
namespace pddl
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Description
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Description::Description(Context &context)
:	m_context(context),
	m_domainPosition{-1},
	m_domain{std::make_unique<Domain>(Domain(m_context))},
	m_problemPosition{-1}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Description Description::fromContext(Context &context)
{
	Description description(context);

	description.parse();

	return description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Description Description::fromStream(std::istream &istream, Context &context)
{
	Description description(context);

	description.m_context.parser.read("std::cin", istream);
	description.parse();

	return description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Description Description::fromFile(const std::string &path, Context &context)
{
	Description description(context);

	description.m_context.parser.read(path);
	description.parse();

	return description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Description Description::fromFiles(const std::vector<std::string> &paths, Context &context)
{
	BOOST_ASSERT(!paths.empty());

	// TODO: handle dirty context objects (for instance, reused context objects in unit tests)

	Description description(context);

	std::for_each(paths.cbegin(), paths.cend(),
		[&](const auto &path)
		{
			description.m_context.parser.read(path);
		});

	description.parse();

	return description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Context &Description::context()
{
	return m_context;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Context &Description::context() const
{
	return m_context;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Domain &Description::domain() const
{
	BOOST_ASSERT(m_domain);

	return *m_domain;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Description::containsProblem() const
{
	return m_problem.get() != nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Problem &Description::problem() const
{
	BOOST_ASSERT(m_problem);

	return *m_problem;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Description::parse()
{
	auto &parser = m_context.parser;

	parser.removeComments(";", "\n", false);

	// First, determine the locations of domain and problem
	findSections();

	if (m_domainPosition == -1)
		throw ConsistencyException("no PDDL domain specified");

	parser.seek(m_domainPosition);
	m_domain->parse();

	if (m_problemPosition != -1)
	{
		parser.seek(m_problemPosition);
		m_problem->parse();
	}

	checkConsistency();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Description::findSections()
{
	auto &parser = m_context.parser;

	parser.skipWhiteSpace();

	while (!parser.atEnd())
	{
		const auto position = parser.position();

		parser.expect<std::string>("(");
		parser.expect<std::string>("define");
		parser.expect<std::string>("(");

		if (parser.testAndSkip<std::string>("domain"))
		{
			if (m_domainPosition != -1)
				throw input::ParserException(parser.location(), "PDDL description may not contain two domains");

			m_domainPosition = position;

			parser.seek(position);
			m_domain->findSections();
		}
		else if (m_context.parser.testAndSkip<std::string>("problem"))
		{
			if (m_problemPosition != -1)
				throw input::ParserException(parser.location(), "PDDL description may currently not contain two problems");

			m_problem = std::make_unique<Problem>(Problem(m_context, *m_domain));

			m_problemPosition = position;

			parser.seek(position);
			m_problem->findSections();
		}
		else
		{
			const auto sectionIdentifier = parser.parse<std::string>();
			throw input::ParserException(parser.location(), "unknown PDDL section “" + sectionIdentifier + "”");
		}

		m_context.parser.skipWhiteSpace();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Description::checkConsistency()
{
	m_domain->checkConsistency();
	m_problem->checkConsistency();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Description::normalize()
{
	m_domain->normalize();

	if (m_problem)
		m_problem->normalize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
