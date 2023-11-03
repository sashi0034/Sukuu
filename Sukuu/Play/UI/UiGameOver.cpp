#include "stdafx.h"
#include "UiGameOver.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "Play/PlayScene.h"
#include "Play/Other/PlayingTips.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Play;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_game_over." + key);
	}
}

struct UiGameOver::Impl
{
	int m_floorIndex{};
	double m_bgAlpha{};

	Vec2 m_mainTextPos{};
	double m_mainTextAlpha{};

	Vec2 m_subTextPos{};
	double m_subTextAlpha{};

	double m_lineWidth{};

	String m_tipsMessage{};
	double m_tipsScale{};

	double m_wholeScale = 1.0;

	void Update()
	{
		if (m_bgAlpha == 0) return;
		(void)Rect(Scene::Size()).draw(ColorF(Constants::HardDarkblue, m_bgAlpha));

		const Transformer2D transform0{Mat3x2::Scale(m_wholeScale, Scene::Center())};

		auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf);
		const Color borderColor = getToml<Color>(U"border_color");
		const Color redColor = getToml<Color>(U"red_color");
		if (m_mainTextAlpha > 0)
		{
			font(U"ゲームオーバー").drawAt(
				TextStyle::Outline(0.2, borderColor),
				getToml<double>(U"main_size"),
				m_mainTextPos,
				ColorF(redColor, m_mainTextAlpha));
		}
		if (m_subTextAlpha > 0)
		{
			font(U"第 {} 層で死んでしまった"_fmt(m_floorIndex))
				.drawAt(TextStyle::Outline(0.1, borderColor),
				        getToml<double>(U"sub_size"),
				        m_subTextPos,
				        redColor);
		}
		if (m_lineWidth != 0)
		{
			Line(Scene::Center().movedBy(-m_lineWidth / 2, 0), Scene::Center().movedBy(m_lineWidth / 2, 0)).draw(
				LineStyle::SquareDot.offset(getToml<double>(U"line_offset_speed") * Scene::Time()),
				10,
				getToml<Color>(U"line_color"));
		}
		if (m_tipsScale > 0)
		{
			const auto tipsCenter = Rect(Scene::Size()).bottomCenter().moveBy(0, getToml<int>(U"tips_y"));
			const Transformer2D t1{Mat3x2::Scale({m_tipsScale, 1}, tipsCenter)};
			font(m_tipsMessage).drawAt(getToml<double>(U"tips_size"),
			                           tipsCenter,
			                           getToml<Color>(U"tips_color"));
		}
	}

	ActorWeak StartPerform(ActorView self)
	{
		return StartCoro(self, [this, self](YieldExtended yield)
		{
			perform(yield, self);
		});
	}

private:
	void perform(YieldExtended& yield, ActorView self)
	{
		AudioAsset(AssetSes::game_over_back).playOneShot();
		yield.WaitForDead(
			AnimateEasing<EaseInSine>(self, &m_bgAlpha, 1.0, 1.5));
		PlayScene::Instance().GetEnemies().KillAll();

		constexpr double appearDuration = 1.0;

		// ゲームオーバー表示
		m_mainTextPos = Scene::Center();
		AnimateEasing<EaseOutBack>(
			self, &m_mainTextPos, m_mainTextPos.movedBy(0, getToml<double>(U"main_offset")), appearDuration);
		AnimateEasing<EaseOutCubic>(self, &m_mainTextAlpha, 1.0, appearDuration);
		yield.WaitForTime(0.5);

		// 死んでしまった表示
		m_subTextPos = Scene::Center();
		AnimateEasing<EaseOutBack>(
			self, &m_subTextPos, m_subTextPos.movedBy(0, getToml<double>(U"sub_offset")), appearDuration);
		AnimateEasing<EaseOutCubic>(self, &m_subTextAlpha, 1.0, appearDuration);
		yield.WaitForTime(0.5);

		// 中央点線
		const double lineWidth = getToml<int>(U"line_width");
		AnimateEasing<EaseOutCubic>(self, &m_lineWidth, lineWidth, appearDuration);
		yield.WaitForTime(1.0);

		// ヒント
		AudioAsset(AssetSes::game_over_front).playOneShot();
		m_tipsMessage = U"[TIPS] " + GetPlayingTips(m_floorIndex);
		AnimateEasing<EaseOutBack>(self, &m_tipsScale, 1.0, appearDuration);

		double waitTime{};
		while (true)
		{
			waitTime += Scene::DeltaTime();
			if (waitTime > 5.0) break;
			if (IsSceneLeftClicked()) break;
			yield();
		}

		yield.WaitForDead(AnimateEasing<EaseInBack>(self, &m_wholeScale, 0.0, 0.3));

		yield.WaitForTime(0.5);
	}
};

namespace Play
{
	UiGameOver::UiGameOver() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiGameOver::Init(int floorIndex)
	{
		p_impl->m_floorIndex = floorIndex;
	}

	void UiGameOver::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double UiGameOver::OrderPriority() const
	{
		return 100000.0;
	}

	ActorWeak UiGameOver::StartPerform()
	{
		return p_impl->StartPerform(*this);
	}
}
