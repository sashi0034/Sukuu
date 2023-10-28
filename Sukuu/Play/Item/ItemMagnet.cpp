#include "stdafx.h"
#include "ItemMagnet.h"

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
		return Util::GetTomlParameter<T>(U"play.item_magnet." + key);
	}
}

struct Play::ItemMagnet::Impl
{
	CharaPosition m_pos{};
	AnimTimer m_animTimer{};

	void Update()
	{
		m_animTimer.Tick();

		m_pos.viewPos = m_pos.actualPos + Circular(
			getToml<int>(U"roll_radius") * (1 + Math::Sin(m_animTimer.Time() * getToml<double>(U"roll_jag"))),
			m_animTimer.Time() * getToml<double>(U"roll_speed"));

		const double lifetime = getToml<double>(U"lifetime");
		const double alpha =
			m_animTimer.Time() > lifetime - 2.0
				? static_cast<int>(m_animTimer.Time() * 1000) % 200 > 100
					  ? 0.1
					  : 0.9
				: 1.0;

		(void)getTexture().drawAt(getDrawPos().movedBy(spriteRect.size / 2), ColorF{1.0, alpha});

		if (m_animTimer.Time() > lifetime)
		{
			PlayScene::Instance().GetGimmick()[m_pos.actualPos.MapPoint()] = GimmickKind::None;
		}
	}

private:
	Vec2 getDrawPos() const
	{
		return m_pos.viewPos.movedBy(GetItemCellPadding(spriteRect.size));
	}

	TexturedQuad getTexture() const
	{
		const double scalePhase = m_animTimer.Time() * getToml<double>(U"scale_speed");
		const double scale = 1 + (1 - Math::Cos(scalePhase)) * getToml<double>(U"scale_rate");

		return TextureAsset(AssetImages::magnet_16x16)(spriteRect.movedBy(
			       m_animTimer.SliceFrames(getToml<double>(U"anim_interval"), 3) * spriteRect.w, 0))
		       .scaled(scale)
		       .rotated(m_animTimer.Time() * getToml<double>(U"rotate_speed"));
	}
};

namespace Play
{
	ItemMagnet::ItemMagnet() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void ItemMagnet::Init(const CharaVec2& pos)
	{
		p_impl->m_pos.SetPos(pos);
		PlayScene::Instance().GetGimmick()[pos.MapPoint()] = GimmickKind::Installed_Magnet;
	}

	void ItemMagnet::Update()
	{
		ActorBase::Update();
		p_impl->Update();

		if (PlayScene::Instance().GetGimmick()[p_impl->m_pos.actualPos.MapPoint()] != GimmickKind::Installed_Magnet)
		{
			Kill();
		}
	}

	double ItemMagnet::OrderPriority() const
	{
		return GetItemCharaOrderPriority(p_impl->m_pos, spriteRect.size);
	}
}
