#include "stdafx.h"
#include "ItemSun.h"

#include "Assets.generated.h"
#include "detail/ItemUtil.h"
#include "Play/PlayCore.h"
#include "Util/EasingAnimation.h"

namespace
{
	using namespace Play::Item_detail;

	constexpr Rect spriteRect{0, 0, 16, 16};

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.item_sun." + key);
	}
}

struct Play::ItemSun::Impl
{
	bool m_killed{};
	CharaPosition m_pos{};
	Dir4Type m_dir{Dir4::Invalid};
	AnimTimer m_animTimer{};
	double m_rotation{};
	ItemAttackerAffair m_attack{ConsumableItem::Pin};
	bool m_blinking{};

	void Update()
	{
		m_animTimer.Tick();

		const double alpha =
			m_blinking
				? static_cast<int>(m_animTimer.Time() * 1000) % 200 > 100
					  ? 0.1
					  : 0.9
				: 1.0;

		(void)TextureAsset(AssetImages::omeme_patchouli_sun_16x16)(spriteRect.movedBy(
			      m_animTimer.SliceFrames(100, 2) * spriteRect.w, 0))
		      .rotatedAt(spriteRect.center(), m_rotation)
		      .draw(getDrawPos(), ColorF{1.0, alpha});

		if (PlayCore::Instance().GetEnemies().SendDamageCollider(
			m_attack, GetItemCollider(m_pos, spriteRect.size)) > 0)
		{
			AudioAsset(AssetSes::attack1).playOneShot();
		}
	}

	void StartFlowchart(ActorView self)
	{
		StartCoro(self, [this,self](YieldExtended yield) mutable
		{
			flowchartLoop(yield, self);
		});
	}

private:
	Vec2 getDrawPos() const
	{
		return m_pos.viewPos.movedBy(GetItemCellPadding(spriteRect.size));
	}

	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		int movedCount{};

		while (true)
		{
			if (CanMoveTo(PlayCore::Instance().GetMap(), m_pos.actualPos, m_dir) == false)
			{
				m_dir = m_dir.RotatedR();
				yield.WaitForExpire(AnimateEasing<EaseOutBack>(
					self,
					&m_rotation,
					m_rotation - Math::TwoPi,
					getToml<double>(U"rotation_speed")));
				continue;
			}
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, getToml<double>(U"move_duration"));
			movedCount++;
			if (movedCount > getToml<int>(U"movable_count")) break;
			if (movedCount > getToml<int>(U"blinking_start")) m_blinking = true;
		}

		// 消滅
		m_killed = true;
		yield();
	}
};

namespace Play
{
	ItemSun::ItemSun() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void ItemSun::Init(const CharaVec2& pos, Dir4Type dir)
	{
		p_impl->m_pos.SetPos(pos.MapPoint() * CellPx_24);
		p_impl->m_dir = dir;
		p_impl->StartFlowchart(*this);
	}

	void ItemSun::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		if (p_impl->m_killed) Kill();
	}

	double ItemSun::OrderPriority() const
	{
		return GetItemCharaOrderPriority(p_impl->m_pos, spriteRect.size);
	}
}
