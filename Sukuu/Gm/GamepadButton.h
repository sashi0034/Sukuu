#pragma once

namespace Gm
{
	enum class GamepadButton
	{
		A,
		B,
		X,
		Y,
		DRight,
		DUp,
		DLeft,
		DDown,
		LB,
		RB,
		LT,
		RT,
		Menu,
		Max
	};

	constexpr int GamepadButtonSize = static_cast<int>(GamepadButton::Max);

	template <typename T>
	class GamepadButtonMapper : public std::array<T, GamepadButtonSize>
	{
	public:
		GamepadButtonMapper() = default;

		GamepadButtonMapper(const Array<int>& array)
		{
			for (int i = 0; i < std::min(this->size(), array.size()); ++i) this->operator[](i) = array[i];
		}

		T& operator[](int button) { return this->std::array<int, GamepadButtonSize>::operator[](button); }
		const T& operator[](int button) const { return this->std::array<int, GamepadButtonSize>::operator[](button); }

		T& operator[](GamepadButton button)
		{
			return this->std::array<int, GamepadButtonSize>::operator[](static_cast<int>(button));
		}

		const T& operator[](GamepadButton button) const
		{
			return this->std::array<int, GamepadButtonSize>::operator[](static_cast<int>(button));
		}
	};

	using GamepadButtonMapping = GamepadButtonMapper<int>;

	constexpr int GamepadPlayer_0 = 0;
}
