﻿#pragma once

#include "Forward.h"
#include "Player_detail\PlayerDefinition.h"

namespace Play
{
	class Player : public ActorBase
	{
	public:
		Player();
		void Init();
		void Update() override;
		double OrderPriority() const override;

		Mat3x2 CameraTransform() const;
		const PlayerDistField& DistField() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}