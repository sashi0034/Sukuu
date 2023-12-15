#include "stdafx.h"
#include "TitleHud.h"

#include "AssetKeys.h"
#include "Constants.h"
#include "GitRevision.h"
#include "Gm/DialogSettingConfigure.h"
#include "Gm/DialogYesNo.h"
#include "Gm/GamepadObserver.h"
#include "Play/Other/CornerButton.h"
#include "Util/Utilities.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"title.hud." + key);
	}

	using Play::CornerButton;
}

struct Title::TitleHud::Impl
{
	String m_record{};
	bool m_showPrompt{};
	double m_promptAnim{};
	bool m_concludedPlay{};
	bool m_concludedRetryTutorial{};
	Array<CornerButton> m_buttons{};
	int m_cursorIndex{};

	void Init(const Gm::GameSavedata& savedata)
	{
		m_record = [&]() -> String
		{
			if (savedata.bestReached == 0) return U"";
			if (savedata.bestReached == Constants::MaxFloorIndex && savedata.completedTime > 0)
			{
				return U"踏破 {}"_fmt(FormatTimeSeconds(savedata.completedTime));
			}
			return U"到達 {} 層"_fmt(savedata.bestReached);
		}();

		m_buttons.push_back(CornerButton(U"終了"_sv, []()
		{
			const ScopedRenderTarget2D rs{none};
			if (Gm::DialogYesNo(U"ゲームを終了しますか") == MessageBoxResult::Yes)
			{
				System::Exit();
			}
		}));

		m_buttons.push_back(CornerButton(U"クレジット"_sv, []()
		{
			System::LaunchFile(U"./credit.html");
		}));

		m_buttons.push_back(CornerButton(U"チュートリアル"_sv, [this]()
		{
			const ScopedRenderTarget2D rs{none};
			if (Gm::DialogYesNo(U"もう一度チュートリアルをしますか") == MessageBoxResult::Yes)
			{
				m_concludedRetryTutorial = true;
			}
		}));

		m_buttons.push_back(CornerButton(U"設定"_sv, []()
		{
			Gm::DialogSettingConfigure();
		}));

		m_cursorIndex = m_buttons.size();
	}

	void Update()
	{
		TextureAsset(AssetKeys::title_logo)
			.scaled(getToml<double>(U"logo_scale"))
			.drawAt(Vec2{Scene::Center().x, Scene::Center().y / 2} + Vec2{
				Periodic::Sine0_1(4.0) * getToml<double>(U"logo_move_x"),
				Periodic::Sine0_1(3.0) * getToml<double>(U"logo_move_y")
			});

		if (not m_record.empty())
		{
			// 記録情報
			(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(m_record)
				.drawAt(TextStyle::Outline(getToml<double>(U"record_outline"), ColorF{0.3}),
				        getToml<double>(U"record_font"),
				        Scene::Center().movedBy(0, getToml<double>(U"record_y")),
				        Palette::Goldenrod);
		}

		// バージョン
		FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"Version git-" + GitRevisionLiteral)
			.draw(Arg::bottomLeft = Scene::Size().y0().yx());

		bool buttonHovered{};
		for (int i = 0; i < m_buttons.size(); ++i)
		{
			// ボタン描画
			buttonHovered |= m_buttons[i].Update(i, m_cursorIndex);
		}

		// 開始可能まで待機
		if (not m_showPrompt) return;

		// 開始プロンプト
		m_promptAnim += 2.0 * Scene::DeltaTime();
		const auto promptMessage = Gm::IsUsingGamepad()
			                           ? U"ダンジョンに潜る"_sv
			                           : U"左クリックでダンジョンに潜る"_sv;
		const auto promptRegion = FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(promptMessage)
			.drawAt(getToml<double>(U"prompt_font"),
			        Scene::Center().movedBy(0, getToml<double>(U"prompt_y")),
			        ColorF(1.0, Math::Abs(Math::Sin(m_promptAnim))));

		// 開始チェック
		if (Gm::IsUsingGamepad())
		{
			const int buttonCount = m_buttons.size() + 1;
			// カーソル移動
			m_cursorIndex = Mod2(
				m_cursorIndex + IsGamepadDown(Gm::GamepadButton::DUp) - IsGamepadDown(Gm::GamepadButton::DDown),
				buttonCount);
			if (m_cursorIndex == m_buttons.size())
			{
				Play::DrawButtonFrame(promptRegion);
				if (IsGamepadDown(Gm::GamepadButton::A)) m_concludedPlay = true;
			}
		}
		else
		{
			if (IsSceneLeftClicked()) m_concludedPlay = true;
		}
	}
};

namespace Title
{
	TitleHud::TitleHud() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TitleHud::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	void TitleHud::Init(const Gm::GameSavedata& savedata)
	{
		p_impl->Init(savedata);
	}

	double TitleHud::OrderPriority() const
	{
		return 1000.0;
	}

	void TitleHud::SetShowPrompt(bool show)
	{
		p_impl->m_showPrompt = show;
	}

	bool TitleHud::IsConcludedPlay() const
	{
		return p_impl->m_concludedPlay;
	}

	bool TitleHud::IsConcludedRetryTutorial() const
	{
		return p_impl->m_concludedRetryTutorial;
	}
}
