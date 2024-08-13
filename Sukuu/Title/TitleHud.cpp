#include "stdafx.h"
#include "TitleHud.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "BuildInformation.h"
#include "Constants.h"
#include "GitRevision.h"
#include "Gm/DialogSettingConfigure.h"
#include "Gm/DialogMessageBox.h"
#include "Gm/GamepadObserver.h"
#include "Gm/LocalizedTextDatabase.h"
#include "Play/PlayingUra.h"
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
	Optional<TitleLogoData> m_fixedLogoData{};
	bool m_showPrompt{};
	double m_promptAnim{};
	bool m_concludedPlay{};
	bool m_concludedRetryTutorial{};
	Array<CornerButton> m_buttons{};
	int m_cursorIndex{};
	Gm::GameSavedata m_saved{};

	void Init(const Gm::GameSavedata& savedata)
	{
		m_saved = savedata;

		m_buttons.push_back(CornerButton(U"exit"_sv, []()
		{
			const ScopedRenderTarget2D rs{none};
			if (Gm::DialogYesNo(U"ask_exit_game"_localize) == MessageBoxResult::Yes)
			{
				System::Exit();
			}
		}));

		m_buttons.push_back(CornerButton(U"credit"_sv, []()
		{
			System::LaunchFile(U"./credit.html");
		}));

		m_buttons.push_back(CornerButton(U"tutorial"_sv, [this]()
		{
			const ScopedRenderTarget2D rs{none};
			if (Gm::DialogYesNo(U"ask_retry_tutorial"_localize) == MessageBoxResult::Yes)
			{
				m_concludedRetryTutorial = true;
			}
		}));

		m_buttons.push_back(CornerButton(U"settings_with_emoji"_sv, []()
		{
			Gm::DialogSettingConfigure();
		}));

		if (savedata.standard.completedTime > 0)
		{
			m_buttons.push_back(CornerButton(U"ura_switch"_sv, []()
			{
				if (Gm::DialogYesNo(U"ask_ura_switch"_localize) == MessageBoxResult::Yes)
				{
					Play::SetPlayingUra(not Play::IsPlayingUra());
				}
			}));
		}

		m_cursorIndex = m_buttons.size();
	}

	void Update()
	{
		TitleLogoData logoData{
			.position = Vec2{Scene::Center().x, Scene::Center().y / 2} + Vec2{
				Periodic::Sine0_1(4.0) * getToml<double>(U"logo_move_x"),
				Periodic::Sine0_1(3.0) * getToml<double>(U"logo_move_y")
			},
			.scale = getToml<double>(U"logo_scale")
		};
		if (m_fixedLogoData.has_value()) logoData = m_fixedLogoData.value();

		// ロゴの描画
		(void)TextureAsset(Play::IsPlayingUra()
			                   ? AssetImages::ura_title_logo
			                   : AssetImages::title_logo)
		      .scaled(logoData.scale)
		      .drawAt(logoData.position);

		// ロゴ作成時は他の UI の描画を行わない
		if (m_fixedLogoData.has_value()) return;

		const String record = getRecordText();
		if (not record.empty())
		{
			// 記録情報
			(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(record)
				.drawAt(TextStyle::Outline(getToml<double>(U"record_outline"), ColorF{0.3}),
				        getToml<double>(U"record_font"),
				        Scene::Center().movedBy(0, getToml<double>(U"record_y")),
				        Palette::Goldenrod);
		}

		// バージョン
		FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"v{}-{}"_fmt(
				BuildInformation::GetRecord().version.ToString(), GitRevisionLiteral))
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
			                           ? U"title_start_play_by_gp"_localize
			                           : U"title_start_play_by_km"_localize;
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
			if (not buttonHovered && IsSceneLeftClicked()) m_concludedPlay = true;
		}
	}

private:
	String getRecordText() const
	{
		const auto& record = m_saved.GetRecord(Play::IsPlayingUra());
		if (record.bestReached == 0) return U"";
		if (record.bestReached == Constants::MaxFloorIndex && record.completedTime > 0)
		{
			return U"title_record_on_50"_localizef(FormatTimeSeconds(record.completedTime));
		}
		return U"title_record_achieved"_localizef(record.bestReached);
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

	void TitleHud::ForceLogoData(const TitleLogoData& data)
	{
		p_impl->m_fixedLogoData = data;
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
