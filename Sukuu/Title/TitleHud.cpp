#include "stdafx.h"
#include "TitleHud.h"

#include "AssetKeys.h"
#include "Constants.h"
#include "GitRevision.h"
#include "Gm/DialogSettingConfigure.h"
#include "Gm/DialogYesNo.h"
#include "Gm/GamepadObserver.h"
#include "Util/Utilities.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"title.hud." + key);
	}

	void drawButton(const Rect& rect, bool hover)
	{
		rect
			.drawShadow({6, 6}, 24, 3)
			.rounded(20).draw(getToml<ColorF>(U"button_color") * (hover ? 1.3 : 1.0));
	};

	void drawButtonFrame(const RectF& region)
	{
		region.stretched(8).rounded(8).drawFrame(4 + 4 * Periodic::Sine0_1(1.0s), Palette::Gold);
	}

	class CornerButton
	{
	public:
		CornerButton(const StringView& label, const std::function<void()>& action) :
			label(label),
			action(action) { return; }

		bool Update(int index, int cursorIndex) const
		{
			// ボタン描画基本処理
			const auto buttonSize = getToml<Size>(U"button_size");
			const int buttonSpace = getToml<int>(U"button_space");
			const auto buttonPadding = getToml<Point>(U"button_padding");

			auto&& exitRect = Rect(
				Scene::Size().moveBy(buttonPadding - buttonSize).movedBy(0, -buttonSpace * index),
				buttonSize);
			const bool exitHover =
				exitRect.intersects(RectF(Arg::center = Cursor::PosF(), Constants::CursorSize_64));
			const bool focused = Gm::IsUsingGamepad()
				                     ? index == cursorIndex
				                     : exitHover;
			drawButton(exitRect, focused);
			(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(label).drawAt(exitRect.center());

			// 入力チェック
			if (focused)
			{
				if (Gm::IsUsingGamepad())
				{
					drawButtonFrame(exitRect);
					if (IsGamepadDown(Gm::GamepadButton::A)) action();
				}
				else
				{
					if (MouseL.down()) action();
				}
			}

			return exitHover;
		}

	private:
		StringView label;
		std::function<void()> action;
	};
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
				drawButtonFrame(promptRegion);
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
