#pragma once

namespace Util
{
	/// @brief If assertion fails, the program stops and shows the message
	/// @return Condition as is
	bool AssertStrongly(bool condition, const std::source_location& location = std::source_location::current());
}
