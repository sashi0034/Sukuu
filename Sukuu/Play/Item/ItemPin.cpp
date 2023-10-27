#include "stdafx.h"
#include "ItemPin.h"

#include "AssetsGenerated.h"
#include "detail/ItemUtil.h"
#include "Play/PlayScene.h"

namespace Play
{
	using namespace Item_detail;

	constexpr Rect spriteRect{0, 0, 16, 16};

	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.item_pin." + key);
	}
}

struct Play::ItemPin::Impl
{
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
		      .draw(m_pos.viewPos.movedBy(GetItemCellPadding(spriteRect.size)));
		PlayScene::Instance().GetEnemies().SendDamageCollider(m_attack, GetItemCollider(m_pos, spriteRect.size));
	}

	void StartFlowchart(ActorBase& self)
	{
		StartCoro(self, [this,self](YieldExtended yield) mutable
		{
			flowchartLoop(yield, self);
		});
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorBase& self)
	{
		// 壁にぶつかるまで進む
		while (true)
		{
			yield();
			if (CanMoveTo(PlayScene::Instance().GetMap(), m_pos.actualPos, m_dir) == false) break;
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, getToml<double>(U"move_duration"));
		}

		// また取れるようにする
		if (m_attack.AttackedCount() == 0)
			PlayScene::Instance().GetGimmick()[m_pos.actualPos.MapPoint()] =
				GimmickKind::Item_Pin;

		// 消滅
		self.Kill();
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
		p_impl->m_pos.SetPos(pos);
		p_impl->m_dir = dir;
		p_impl->StartFlowchart(*this);
	}

	void ItemPin::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double ItemPin::OrderPriority() const
	{
		return GetItemCharaOrderPriority(p_impl->m_pos, spriteRect.size);
	}
}
