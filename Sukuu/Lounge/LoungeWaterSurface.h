#pragma once

namespace Lounge
{
	class LoungeWaterSurface
	{
	public:
		LoungeWaterSurface();

		struct render_args
		{
			Vec2 cameraPos;
		};

		void Render(const render_args& args);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
