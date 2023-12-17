#pragma once

namespace Gm
{
	[[nodiscard]]
	MessageBoxResult DialogYesNo(StringView message);

	MessageBoxResult DialogOk(StringView message);
}
