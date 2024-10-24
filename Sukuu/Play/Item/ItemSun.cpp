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
	double m_targetRotation{};

	ItemAttackerAffair m_attack{ConsumableItem::Pin};
	bool m_blinking{};

	Trail m_trail{};

	void Update()
	{
		m_animTimer.Tick();

		m_rotation = Math::Lerp(m_rotation, m_targetRotation, 10.0 * GetDeltaTime());
		const double alpha =
			m_blinking
				? static_cast<int>(m_animTimer.Time() * 1000) % 200 > 100
					  ? 0.1
					  : 0.9
				: 1.0;

		const ScopedColorMul2D mul2d{ColorF{1.0, alpha}};;

		updateTrail();

		(void)TextureAsset(AssetImages::omeme_patchouli_sun_16x16)(spriteRect.movedBy(
			      m_animTimer.SliceFrames(100, 2) * spriteRect.w, 0))
		      .rotatedAt(spriteRect.center(), m_rotation)
		      .draw(getDrawPos());

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

	void updateTrail()
	{
		const auto trailPosition =
			getDrawPos() + Vec2::One() * spriteRect.size / 2;
		m_trail.add(trailPosition, ColorF{1, 0.62, 0.03}, 12.0 + 8.0 * Periodic::Sine1_1(0.1s, m_animTimer.Time()));
		m_trail.update(GetDeltaTime());
		m_trail.draw();
	}

	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		int movedCount{};

		while (true)
		{
			constexpr double moveUnit = CellPx_24 / 4.0;
			const double aheadAmount = (movedCount > 0 ? moveUnit * 1.5 : CellPx_24); // 初回移動時だけ遠めをチェックする
			const auto nextPoint =
				((m_pos.actualPos + Vec2::One() * CellPx_24 / 2 + m_dir.ToXY() * aheadAmount) / CellPx_24).asPoint();

			if (CanMovePointAt(PlayCore::Instance().GetMap(), nextPoint) == false)
			{
				m_dir = m_dir.RotatedR();

				yield();
				continue;
			}

			m_targetRotation = -m_dir.GetIndex() * Math::TwoPi;

			const auto nextPos = m_pos.actualPos + m_dir.ToXY() * moveUnit;
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
