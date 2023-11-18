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
		explicit constexpr EnumValue(T v) : m_value{v} { return; }
		constexpr operator T() const { return m_value; }
		T value() const { return m_value; }

	private:
		T m_value;
	};

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

	inline bool IsSceneLeftClicked(int padding = -32)
	{
		return MouseL.down() && Rect(Scene::Size()).stretched(padding).intersects(Cursor::Pos());
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
}
