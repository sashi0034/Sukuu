#include "stdafx.h"
#include "DialogSettingConfigure.h"

#include "GameConfig.h"
#include "detail/GameDialogCommon.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Gm;
	using namespace Gm::detail;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"setting_configure." + key);
	}

	struct InternalState
	{
		GameConfig editing;
		double passedStarted{};
		double passedFinished{};
	};

	void updateUi(GameConfig& editing, bool* exitHover)
	{
		DrawDialogTitle(U"設定");

		DrawDialogExit(exitHover);
	}

	GameConfig loopInternal(const GameConfig& current)
	{
		InternalState state{};
		state.editing = current;

		bool exitRequested{};

		while (System::Update())
		{
			Transformer2D transformer2D{PreUpdateDialog(state.passedStarted, state.passedFinished, true)};

			state.passedStarted += Scene::DeltaTime();
			if (exitRequested) state.passedFinished += Scene::DeltaTime();

			bool exitHover;
			updateUi(state.editing, &exitHover);

			exitRequested = exitRequested || CheckDialogExit(exitHover, true);

			if (IsFinishDialog(state.passedFinished)) break;
		}

		return state.editing;
	}
}

GameConfig Gm::DialogSettingConfigure(const GameConfig& current)
{
	const auto bg = Util::ScopedBackgroundStore();
	const auto result = loopInternal(current);
	return result;
}
