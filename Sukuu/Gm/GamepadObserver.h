#pragma once
#include "Util/ActorBase.h"

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

	bool IsUsingGamepad();

	bool IsGamepadPressed(GamepadButton button);
	bool IsGamepadUp(GamepadButton button);
	bool IsGamepadDown(GamepadButton button);

	template <typename T>
	class GamepadButtonMap : public std::array<T, GamepadButtonSize>
	{
	public:
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

	class GamepadObserver : public Util::ActorBase
	{
	public:
		GamepadObserver();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
