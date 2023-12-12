#include "stdafx.h"
#include "TitleHud.h"

#include "AssetKeys.h"
#include "Constants.h"
#include "GitRevision.h"
#include "Util/Utilities.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"title.hud." + key);
	}
}

struct Title::TitleHud::Impl
{
	String m_record{};
	bool m_showPrompt{};
	double m_promptAnim{};
	bool m_buttonHovered{};
	bool m_concludedRetryTutorial{};

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
	}

	void Update()
	{
		TextureAsset(AssetKeys::title_logo)
			.scaled(getToml<double>(U"logo_scale"))
			.drawAt(Vec2{Scene::Center().x, Scene::Center().y / 2} + Vec2{
				Periodic::Sine0_1(4.0) * getToml<double>(U"logo_move_x"),
				Periodic::Sine0_1(3.0) * getToml<double>(U"logo_move_y")
			});

		FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"Version git-" + GitRevisionLiteral)
			.draw(Arg::bottomLeft = Scene::Size().y0().yx());

		(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"左クリックを押してください")
			.drawAt(getToml<double>(U"prompt_font"),
			        Scene::Center().movedBy(0, getToml<double>(U"prompt_y")),
			        ColorF(1.0, Math::Abs(Math::Sin(m_promptAnim))));

		if (not m_record.empty())
		{
			(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(m_record)
				.drawAt(TextStyle::Outline(getToml<double>(U"record_outline"), ColorF{0.3}),
				        getToml<double>(U"record_font"),
				        Scene::Center().movedBy(0, getToml<double>(U"record_y")),
				        Palette::Goldenrod);
		}

		if (m_showPrompt)
		{
			m_promptAnim += 2.0 * Scene::DeltaTime();
			(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"左クリックを押してください")
				.drawAt(getToml<double>(U"prompt_font"),
				        Scene::Center().movedBy(0, getToml<double>(U"prompt_y")),
				        ColorF(1.0, Math::Abs(Math::Sin(m_promptAnim))));
		}

		auto&& buttonSize = getToml<Size>(U"button_size");
		const auto buttonDraw = [](const Rect& rect, bool hover)
		{
			rect
				.drawShadow({6, 6}, 24, 3)
				.rounded(20).draw(getToml<ColorF>(U"button_color") * (hover ? 1.3 : 1.0));
		};

		const int buttonSpace = getToml<int>(U"button_space");

		auto&& exitRect = Rect(Scene::Size().moveBy(getToml<Point>(U"button_padding") - buttonSize), buttonSize);
		const bool exitHover =
			exitRect.intersects(RectF(Arg::center = Cursor::PosF(), Constants::CursorSize_64));
		buttonDraw(exitRect, exitHover);
		(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"終了").drawAt(exitRect.center());
		if (exitHover && MouseL.down())
		{
			const ScopedRenderTarget2D rs{none};
			if (System::MessageBoxYesNo(U"ゲームを終了しますか", MessageBoxStyle::Question) == MessageBoxResult::Yes)
			{
				System::Exit();
			}
		}

		auto&& creditRect = exitRect.movedBy(0, -buttonSpace);
		const bool creditHover = not exitHover &&
			creditRect.intersects(RectF(Arg::center = Cursor::PosF(), Constants::CursorSize_64));
		buttonDraw(creditRect, creditHover);
		(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"クレジット").drawAt(creditRect.center());
		if (creditHover && MouseL.down())
		{
			System::LaunchFile(U"./credit.html");
		}

		auto&& tutorialRect = creditRect.movedBy(0, -buttonSpace);
		const bool tutorialHover = not creditHover &&
			tutorialRect.intersects(RectF(Arg::center = Cursor::PosF(), Constants::CursorSize_64));
		buttonDraw(tutorialRect, tutorialHover);
		(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"チュートリアル").drawAt(tutorialRect.center());
		if (tutorialHover && MouseL.down())
		{
			const ScopedRenderTarget2D rs{none};
			if (System::MessageBoxYesNo(U"もう一度チュートリアルをしますか", MessageBoxStyle::Question) == MessageBoxResult::Yes)
			{
				m_concludedRetryTutorial = true;
			}
		}

		m_buttonHovered = exitHover || creditHover || tutorialHover;
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

	bool TitleHud::IsButtonHovered() const
	{
		return p_impl->m_buttonHovered;
	}

	bool TitleHud::IsConcludedRetryTutorial() const
	{
		return p_impl->m_concludedRetryTutorial;
	}
}
