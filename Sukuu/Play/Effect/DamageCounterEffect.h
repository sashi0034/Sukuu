#pragma once

namespace Play
{
	struct DamageCounterEmission
	{
		Vec2 center;
		int amount;
		Color color;
	};

	std::unique_ptr<IEffect> EmitDamageCounterEffect(const DamageCounterEmission& props);
}
