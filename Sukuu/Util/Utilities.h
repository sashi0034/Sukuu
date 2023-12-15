#pragma once

namespace Util
{
	void SetTimeScale(double scale);
	double GetTimeScale();

	void RefreshDeltaTime();
	double GetDeltaTime();

	template <typename T>
	class EnumValue
	{
		static_assert(std::is_integral<T>::value);

	public:
		constexpr EnumValue(T v) : m_value{v} { return; }
		constexpr operator T() const { return m_value; }
		T value() const { return m_value; }

	private:
		T m_value;
	};

	template <typename T>
	T Mod2(T value, T mod)
	{
		return ((value % mod) + mod) % mod;
	}

	template <typename Enum, Enum maxEnum, typename Value>
	Enum AddModuloEnum(Enum e, Value add)
	{
		static_assert(std::is_enum<Enum>::value, "Enum must be an enumeration type.");
		constexpr Value maxValue = static_cast<Value>(maxEnum);
		return Enum(Mod2<Value>(add + static_cast<Value>(e), maxValue));
	}

	template <typename FuncType>
	std::function<FuncType> EmptyLambda()
	{
		return [](auto&&... args) -> decltype(auto)
		{
			(void)sizeof...(args);
			if constexpr (!std::is_void_v<decltype(std::declval<FuncType>()(std::declval<decltype(args)>()...))>)
			{
				return decltype(std::declval<FuncType>()(std::declval<decltype(args)>()...)){};
			}
		};
	}

	inline bool IsSceneLeftClicked(int padding = 32)
	{
		return MouseL.down() && Rect(Scene::Size()).stretched(-padding).intersects(Cursor::Pos());
	}

	inline String FormatTimeSeconds(double t)
	{
		return U"{:02d}:{:02d}.{:02d}"_fmt(
			static_cast<int>(t / 60),
			static_cast<int>(t) % 60,
			(static_cast<int>(t * 1000) % 1000) / 10);
	}

	template <typename T>
	T SumArrayValues(std::span<T const> s)
	{
		return std::accumulate(s.begin(), s.end(), T{});
	}

	template <typename T, typename Rollback = std::function<void(T)>>
	class ScopedValueStore : public Uncopyable
	{
	public:
		[[nodiscard]]
		ScopedValueStore(const T& current, Rollback rollback) :
			m_before(current),
			m_rollback(std::move(rollback))
		{
		}

		~ScopedValueStore()
		{
			m_rollback(m_before);
		}

	private:
		T m_before;
		Rollback m_rollback;
	};

	class ScopedBackgroundStore : public ScopedValueStore<ColorF>
	{
	public:
		[[nodiscard]]
		ScopedBackgroundStore() : ScopedValueStore(Scene::GetBackground(), Scene::SetBackground)
		{
		}
	};
}

#define SINGLETON_SIDEEFFECT
