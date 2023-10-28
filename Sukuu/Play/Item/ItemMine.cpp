#include "stdafx.h"
#include "ItemMine.h"

#include "Assets.generated.h"
#include "detail/ItemUtil.h"
#include "Play/PlayScene.h"

namespace
{
	using namespace Play::Item_detail;

	constexpr Rect spriteRect{0, 0, 16, 16};

	constexpr double animDuration = 0.1;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.item_mine." + key);
	}
}

struct Play::ItemMine::Impl
{
	bool m_killed{};
	CharaPosition m_pos{};
	AnimTimer m_animTimer{};
	double m_passed{};

	void Update()
	{
		m_passed += GetDeltaTime();
		m_animTimer.Tick();

		const int animInterval = getToml<int>(U"anim_interval");
		const double grewTime = m_passed - animInterval * 6 / 1000.0;

		const double scalePhase = ToRadians(grewTime * getToml<double>(U"scale_speed"));
		const double scale =
			grewTime > 0
				? 1 + (1 - Math::Cos(scalePhase)) * getToml<double>(U"scale_rate")
				: 1;

		(void)TextureAsset(AssetImages::mine_flower_16x16)(spriteRect.movedBy(
			      m_animTimer.SliceFrames(animInterval, 6) * spriteRect.w, (grewTime > 0) * spriteRect.h))
		      .scaled(scale)
		      .draw(Arg::bottomCenter = getDrawPos() + spriteRect.bottomCenter());
	}

private:
	Vec2 getDrawPos() const
	{
		return m_pos.viewPos.movedBy(GetItemCellPadding(spriteRect.size));
	}
};

namespace Play
{
	ItemMine::ItemMine() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void ItemMine::Init(const CharaVec2& pos)
	{
		p_impl->m_pos.SetPos(pos);
		PlayScene::Instance().GetGimmick()[pos.MapPoint()] = GimmickKind::Installed_Mine;
	}

	void ItemMine::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		// エネミー側で踏んだ時にギミックを変える
		if (PlayScene::Instance().GetGimmick()[p_impl->m_pos.actualPos.MapPoint()] != GimmickKind::Installed_Mine)
		{
			Kill();
		}
	}

	double ItemMine::OrderPriority() const
	{
		return GetItemCharaOrderPriority(p_impl->m_pos, spriteRect.size);
	}
}
