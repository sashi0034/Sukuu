#pragma once
#include "GamepadButton.h"
#include "Util/ActorBase.h"

namespace Gm
{
	bool IsUsingGamepad();

	/// @brief Input で取得した値を保存する構造体
	struct RetrievedInput
	{
		bool isDown;
		bool isPressed;
		bool isUp;

		bool down() const { return isDown; }
		bool pressed() const { return isPressed; }
		bool up() const { return isUp; }

		[[nodiscard]]
		friend RetrievedInput operator ||(Input left, RetrievedInput right) noexcept;

		static RetrievedInput From(const Input& input);
	};

	RetrievedInput GetGamepadInput(GamepadButton button);
	bool IsGamepadPressed(GamepadButton button);
	bool IsGamepadUp(GamepadButton button);
	bool IsGamepadDown(GamepadButton button);

	// Vec2 GetGamepadAxeL();

	void RefreshGamepad();

	class GamepadObserver : public Util::ActorBase
	{
	public:
		GamepadObserver();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};

	void InitXInputWatcherAddon();
}
