#include "stdafx.h"
#include "EnSlimeCat.h"

#include "Play/PlayScene.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	constexpr Rect catRect{0, 0, 24, 24};
}

struct Play::EnSlimeCat::Impl
{
	CharaPosition m_pos;
	AnimTimer m_animTimer;
	double m_speed = 1.0;
	Dir4Type m_direction{Dir4::Down};

	void Update()
	{
		m_animTimer.Tick();
		getTexture().draw(m_pos.viewPos.movedBy(GetCharacterCellPadding(catRect.size)));
	}

	void FlowchartAsync(YieldExtended& yield, ActorBase& self)
	{
		while (true)
		{
			flowchartLoop(yield, self);
		}
	}

private:
	TextureRegion getTexture() const
	{
		auto&& sheet = TextureAsset(AssetImages::punicat_24x24);
		const int interval = GetTomlParameter<int>(U"play.en_slime_cat.anim_interval");

		switch (m_direction.GetIndex())
		{
		case Dir4Type::Right:
			return sheet(catRect.movedBy(catRect.size * Point{m_animTimer.SliceFrames(interval, 4), 3}));
		case Dir4Type::Up:
			return sheet(catRect.movedBy(catRect.size * Point{m_animTimer.SliceFrames(interval, 4), 2}));
		case Dir4Type::Left:
			return sheet(catRect.movedBy(catRect.size * Point{m_animTimer.SliceFrames(interval, 4), 1}));
		case Dir4Type::Down:
			return sheet(catRect.movedBy(catRect.size * Point{m_animTimer.SliceFrames(interval, 4), 0}));
		default: ;
			return {};
		}
	}

	void flowchartLoop(YieldExtended& yield, ActorBase& self)
	{
		yield();

		while (CanMoveTo(PlayScene::Instance().GetMap(), m_pos.actualPos, m_direction))
		{
			auto nextPos = m_pos.actualPos + m_direction.ToXY() * CellPx_24;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos,
			                    GetTomlParameter<double>(U"play.en_slime_cat.move_duration"));
		}

		m_direction = Dir4Type((m_direction + 1) % 4);
	}
};

namespace Play
{
	EnSlimeCat::EnSlimeCat() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EnSlimeCat::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayScene::Instance().GetMap()));

		StartCoro(*this, [*this](YieldExtended yield) mutable
		{
			p_impl->FlowchartAsync(yield, *this);
		});
	}

	void EnSlimeCat::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double EnSlimeCat::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}
}
