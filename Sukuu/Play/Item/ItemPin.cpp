#include "stdafx.h"
#include "ItemPin.h"

#include "Assets.generated.h"
#include "detail/ItemUtil.h"
#include "Play/PlayCore.h"
#include "Play/Effect/FragmentTextureEffect.h"

namespace
{
	using namespace Play::Item_detail;

	constexpr Rect spriteRect{0, 0, 16, 16};

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.item_pin." + key);
	}
}

struct Play::ItemPin::Impl
{
	bool m_killed{};
	CharaPosition m_pos{};
	Dir4Type m_dir{Dir4::Invalid};
	AnimTimer m_animTimer{};
	double m_rotation{};
	ItemAttackerAffair m_attack{ConsumableItem::Pin};

	void Update()
	{
		m_animTimer.Tick();
		m_rotation += GetDeltaTime() * getToml<double>(U"rotation_speed");
		(void)TextureAsset(AssetImages::pin_16x16)(spriteRect.movedBy(
			      m_animTimer.SliceFrames(200, 3) * spriteRect.w, 0))
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

	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		// 壁にぶつかるまで進む
		while (true)
		{
			yield();
			if (CanMoveTo(PlayCore::Instance().GetMap(), m_pos.actualPos, m_dir) == false) break;
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, getToml<double>(U"move_duration"));
		}

		if (m_attack.AttackedCount() == 0)
		{
			// また取れるようにする
			auto& gimmick = PlayCore::Instance().GetGimmick();
			if (gimmick[m_pos.actualPos.MapPoint()] == GimmickKind::None)
			{
				gimmick[m_pos.actualPos.MapPoint()] = GimmickKind::Item_Pin;
			}
		}
		else
		{
			// エフェクト
			PlayCore::Instance().FgEffect().add(EmitFragmentTextureEffect(
				getDrawPos().moveBy(spriteRect.size / 2),
				TextureAsset(AssetImages::pin_16x16)(spriteRect),
				Palette::Bisque, 64));
		}

		// 消滅
		m_killed = true;
	}
};

namespace Play
{
	ItemPin::ItemPin() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void ItemPin::Init(const CharaVec2& pos, Dir4Type dir)
	{
		p_impl->m_pos.SetPos(pos.MapPoint() * CellPx_24);
		p_impl->m_dir = dir;
		p_impl->StartFlowchart(*this);
	}

	void ItemPin::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		if (p_impl->m_killed) Kill();
	}

	double ItemPin::OrderPriority() const
	{
		return GetItemCharaOrderPriority(p_impl->m_pos, spriteRect.size);
	}
}
