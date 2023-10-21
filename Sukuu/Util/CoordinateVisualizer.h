#pragma once

namespace Util
{
	class CoordinateVisualizer
	{
	public:
		CoordinateVisualizer();

		void Update();

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
