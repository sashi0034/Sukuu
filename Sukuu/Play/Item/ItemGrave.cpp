#include "stdafx.h"
#include "ItemGrave.h"

#include "Assets.generated.h"
#include "detail/ItemUtil.h"
#include "Play/PlayScene.h"

namespace
{
	using namespace Play::Item_detail;

	constexpr Rect spriteRect{0, 0, 16, 16};

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.item_grave." + key);
	}
}

struct Play::ItemGrave::Impl
{
	CharaPosition m_pos{};
	AnimTimer m_animTimer{};

	void Update()
	{
		m_animTimer.Tick();

		const double lifetime = getToml<double>(U"lifetime");
		const double alpha =
			m_animTimer.Time() > lifetime - 2.0
				? static_cast<int>(m_animTimer.Time() * 1000) % 200 > 100
					  ? 0.1
					  : 0.9
				: 1.0;

		const double scalePhase = m_animTimer.Time() * getToml<double>(U"scale_speed");
		const double scale = 1 + (1 - Math::Cos(scalePhase)) * getToml<double>(U"scale_rate");

		static const Array<int> pattern = {0, 1, 2, 3, 4, 0, 0, 0, 0, 0};
		(void)TextureAsset(AssetImages::grave_16x16)(
			      Point{m_animTimer.SliceFrames(getToml<double>(U"anim_interval"), pattern), 0} * spriteRect.size,
			      spriteRect.size)
		      .scaled(scale)
		      .draw(Arg::bottomCenter = getDrawPos().movedBy(spriteRect.size.x / 2, spriteRect.size.y),
		            ColorF{1.0, alpha});

		if (m_animTimer.Time() > lifetime)
		{
			PlayScene::Instance().GetGimmick()[m_pos.actualPos.MapPoint()] = GimmickKind::None;
		}
	}

private:
	Vec2 getDrawPos() const
	{
		return m_pos.viewPos.movedBy(GetCharacterCellPadding(spriteRect.size));
	}
};

namespace Play
{
	ItemGrave::ItemGrave() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void ItemGrave::Init(const CharaVec2& pos)
	{
		p_impl->m_pos.SetPos(pos);
		PlayScene::Instance().GetGimmick()[pos.MapPoint()] = GimmickKind::Installed_Grave;
	}

	void ItemGrave::Update()
	{
		ActorBase::Update();
		p_impl->Update();

		if (PlayScene::Instance().GetGimmick()[p_impl->m_pos.actualPos.MapPoint()] != GimmickKind::Installed_Grave)
		{
			Kill();
		}
	}

	double ItemGrave::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}
}
