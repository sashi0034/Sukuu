#include "stdafx.h"
#include "DamageCounterEffect.h"

#include "AssetKeys.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Play;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.damage_counter_effect." + key);
	}

	struct DamageCounterEffect : IEffect
	{
		DamageCounterEmission m_e{};
		String m_text{};

		explicit DamageCounterEffect(const DamageCounterEmission& e) :
			m_e(e)
		{
			m_text = e.amount > 0
				         ? U"+{}"_fmt(e.amount)
				         : Format(e.amount);
		}

		bool update(double timeSec) override
		{
			const ScopedRenderStates2D state{SamplerState::ClampLinear};

			auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf_Bold);

			const double appearTime = getToml<double>(U"appear_time");
			const double appearRate = timeSec / appearTime;
			const double scale = (timeSec < appearTime ? EaseOutBack(appearRate) : 1.0);
			const double offset = getToml<double>(U"offset") * scale;
			constexpr double endTime = 2.0;
			constexpr double fadeTime = 0.3;
			const double fadeRate = (timeSec < endTime - fadeTime)
				                        ? 0.0
				                        : ((timeSec - (endTime - fadeTime)) / fadeTime);;
			const double fadeOffset = EaseInCirc(fadeRate) * getToml<double>(U"fade_offset");

			(void)font(m_text)
				.drawAt(TextStyle::Outline(getToml<double>(U"outline"),
				                           ColorF{0.2}),
				        getToml<int>(U"font_size") * scale,
				        m_e.center.movedBy(0, offset + fadeOffset),
				        ColorF(m_e.color, 1.0 - fadeRate));

			return timeSec < endTime;
		}
	};
}

namespace Play
{
	std::unique_ptr<IEffect> EmitDamageCounterEffect(const DamageCounterEmission& props)
	{
		return std::make_unique<DamageCounterEffect>(props);
	}
}
