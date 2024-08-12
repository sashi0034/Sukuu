#pragma once

namespace Ending
{
	struct EndingOpenTransitionArgs
	{
		double basicDuration;
		ColorF fg;
	};

	[[nodiscard]]
	std::unique_ptr<IEffect> CreateEndingOpenTransition(const EndingOpenTransitionArgs& args);
}
