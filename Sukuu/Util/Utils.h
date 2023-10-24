#pragma once

namespace Util
{
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
}
