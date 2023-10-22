#pragma once

#include "Forward.h"

namespace Play
{
	class Player : public ActorBase
	{
	public:
		Player();
		void Init();
		void Update() override;

		Mat3x2 CameraTransform() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
