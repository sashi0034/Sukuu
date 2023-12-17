#pragma once

namespace Gm
{
	[[nodiscard]]
	MessageBoxResult DialogYesNo(StringView message);

	[[nodiscard]]
	MessageBoxResult DialogOk(StringView message);
}
