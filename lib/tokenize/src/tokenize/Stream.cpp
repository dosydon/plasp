#include <tokenize/Stream.h>

#include <fstream>

namespace tokenize
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Stream
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Stream::Stream()
{
	std::setlocale(LC_NUMERIC, "C");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Stream::Stream(std::string streamName, std::istream &istream)
{
	read(streamName, istream);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Stream::read(std::string streamName, std::istream &istream)
{
	// Store position of new section
	m_delimiters.push_back({m_stream.size(), streamName});

	try
	{
		istream.seekg(0, std::ios::end);
		const auto streamSize = istream.tellg();
		istream.seekg(0, std::ios::beg);

		const auto startPosition = m_stream.size();

		m_stream.resize(m_stream.size() + streamSize);
		std::copy(std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>(), m_stream.begin() + startPosition);
	}
	catch (const std::exception &exception)
	{
		istream.clear();
		std::copy(std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>(), std::back_inserter(m_stream));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Stream::read(const std::experimental::filesystem::path &path)
{
	if (!std::experimental::filesystem::is_regular_file(path))
		throw std::runtime_error("File does not exist: “" + path.string() + "”");

	std::ifstream fileStream(path.string(), std::ios::in);

	read(path.string(), fileStream);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Stream::reset()
{
	m_position = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Stream::seek(StreamPosition position)
{
	m_position = position;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

StreamPosition Stream::position() const
{
	return m_position;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
