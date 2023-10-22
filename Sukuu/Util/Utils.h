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
}
