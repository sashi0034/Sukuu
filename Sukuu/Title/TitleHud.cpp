#include "stdafx.h"
#include "TitleHud.h"

#include "AssetKeys.h"
#include "GitRevision.h"

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
	bool m_creditHovered{};

	void Init()
	{
		m_record = U"到達 12 層";
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

		(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"右クリックを押してください")
			.drawAt(getToml<double>(U"prompt_font"),
			        Scene::Center().movedBy(0, getToml<double>(U"prompt_y")),
			        ColorF(1.0, Math::Abs(Math::Sin(m_promptAnim))));

		(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(m_record)
			.drawAt(TextStyle::Outline(getToml<double>(U"record_outline"), ColorF{0.3}),
			        getToml<double>(U"record_font"),
			        Scene::Center().movedBy(0, getToml<double>(U"record_y")),
			        Palette::Goldenrod);

		if (m_showPrompt)
		{
			m_promptAnim += 2.0 * Scene::DeltaTime();
			(void)FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"右クリックを押してください")
				.drawAt(getToml<double>(U"prompt_font"),
				        Scene::Center().movedBy(0, getToml<double>(U"prompt_y")),
				        ColorF(1.0, Math::Abs(Math::Sin(m_promptAnim))));
		}

		auto&& creditSize = getToml<Size>(U"credit_size");
		const auto creditRect = Rect(Scene::Size().moveBy(getToml<Point>(U"credit_padding") - creditSize), creditSize);
		m_creditHovered = creditRect.intersects(Cursor::PosF());
		(void)creditRect
		      .drawShadow({6, 6}, 24, 3)
		      .rounded(20).draw(getToml<ColorF>(U"credit_color") * (m_creditHovered ? 1.3 : 1.0));
		(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"クレジット").drawAt(creditRect.center());
		if (m_creditHovered && MouseL.down())
		{
			System::LaunchFile(U"./credit.html");
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

	void TitleHud::Init()
	{
		p_impl->Init();
	}

	double TitleHud::OrderPriority() const
	{
		return 1000.0;
	}

	void TitleHud::SetShowPrompt(bool show)
	{
		p_impl->m_showPrompt = show;
	}

	bool TitleHud::IsCreditHovered() const
	{
		return p_impl->m_creditHovered;
	}
}
