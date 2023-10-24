#include "stdafx.h"
#include "EnSlimeCat.h"

#include "AssetsGenerated.h"
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
	int m_playerFollowing = 0;

	void Update()
	{
		// プレイヤーとの当たり判定
		auto&& player = PlayScene::Instance().GetPlayer();
		if (player.DistField()[m_pos.actualPos.MapPoint()].distance != PlayerDistanceInfinity)
		{
			player.SendEnemyCollide({m_pos.actualPos, catRect.size});
		}

		// アニメーション更新
		m_animTimer.Tick();
		const auto drawingPos = m_pos.viewPos.movedBy(GetCharacterCellPadding(catRect.size));
		(void)getTexture().draw(drawingPos);

		// 吹き出し描画
		if (m_playerFollowing > 0)
		{
			ScopedRenderStates2D sampler{SamplerState::ClampLinear};
			const auto drawingRect = RectF{
				drawingPos.movedBy(0, GetTomlParameter<int>(U"play.en_slime_cat.baloon_offset_y")), catRect.size
			};

			(void)Shape2D::RectBalloon(drawingRect,
			                           drawingPos.movedBy(catRect.size.x / 2 - 1, 0),
			                           0.5)
				.draw();

			(void)TextureAsset(U"😎").resized(drawingRect.size).draw(drawingRect.pos);
		}
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

	void checkFollowPlayer(YieldExtended& yield, const Point currentPoint)
	{
		auto&& playerDf = PlayScene::Instance().GetPlayer().DistField();
		const int currentDist = playerDf[currentPoint].distance;
		const bool nearPlayer =
			playerDf[currentPoint].directStraight // 現在の点がプレイヤー十字上
			|| playerDf[currentPoint + m_direction.ToXY().asPoint()].directStraight; // 次の点がプレイヤー十字上

		const auto nextDist = playerDf[currentPoint + m_direction.ToXY().asPoint()].distance;

		if (nextDist < currentDist && (nearPlayer || m_playerFollowing > 0))
		{
			// プレイヤーに近づいている
			m_playerFollowing = GetTomlParameter<int>(U"play.en_slime_cat.player_followiung_intensity");
		}
		if (nextDist > currentDist || m_playerFollowing > 0)
		{
			// 追跡中だけど、プレイヤーから遠ざかっている
			m_playerFollowing--;

			// プレイヤー見失った
			if (m_playerFollowing == 0)
			{
				// ペナルティ
				yield.WaitForTime(GetTomlParameter<double>(U"play.en_slime_cat.lost_player_penalty"));
			}
		}

		// プレイヤーが近くにいない
		if (nearPlayer == false || m_playerFollowing == 0) return;

		const int backDist = playerDf[currentPoint + m_direction.Reversed().ToXY().asPoint()].distance;
		// プレイヤーが後ろ向きにいないなら追跡
		if (backDist >= currentDist)
		{
			// このまま正面に進むとプレイヤーから離れてしまうとき
			if (nextDist > currentDist)
			{
				if (playerDf[currentPoint + m_direction.RotatedL().ToXY().asPoint()].distance < currentDist)
				{
					m_direction = m_direction.RotatedL();
				}
				else if (playerDf[currentPoint + m_direction.RotatedR().ToXY().asPoint()].distance < currentDist)
				{
					m_direction = m_direction.RotatedR();
				}
			}
		}
		else
		{
			// 後ろにいる
			m_direction = m_direction.Reversed();
		}
	}

	void flowchartLoop(YieldExtended& yield, ActorBase& self)
	{
		yield();
		const bool preRandomBool = Random(0, 1);
		int proceededCount = 0;
		while (true)
		{
			auto&& map = PlayScene::Instance().GetMap();
			const TerrainKind currentTerrain = GetTerrainAt(map, m_pos.actualPos);
			const auto currentPoint = m_pos.actualPos.MapPoint();

			// プレイヤー追跡チェック
			checkFollowPlayer(yield, currentPoint);

			// 曲がるかチェック
			if (checkTurn(map, currentTerrain, proceededCount, preRandomBool)) break;

			// 直線方向に進めるかチェック
			if (CanMoveTo(map, m_pos.actualPos, m_direction) == false)
			{
				// 直進できないなら、曲がれるかチェック
				if (CanMoveTo(map, m_pos.actualPos, m_direction.Rotated(preRandomBool)))
				{
					m_direction = m_direction.Rotated(preRandomBool);
				}
				else if (CanMoveTo(map, m_pos.actualPos, m_direction.Rotated(not preRandomBool)))
				{
					m_direction = m_direction.Rotated(not preRandomBool);
				}
				else
				{
					m_direction = m_direction.Reversed();
					break;
				}
			}

			// 直線方向に進む
			auto nextPos = m_pos.actualPos + m_direction.ToXY() * CellPx_24;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos,
			                    GetTomlParameter<double>(U"play.en_slime_cat.move_duration"));
			proceededCount++;
		}
	}

	bool checkTurn(const MapGrid& map, const TerrainKind currentTerrain, int proceededCount, const bool preRandomBool)
	{
		// 部屋から出れるかチェック
		if (proceededCount > 0 &&
			currentTerrain == TerrainKind::Floor &&
			Random(0, GetTomlParameter<int>(U"play.en_slime_cat.floor_turn_likely")) != 0)
		{
			if (GetTerrainFor(map, m_pos.actualPos, m_direction.Rotated(preRandomBool)) == TerrainKind::Pathway)
			{
				m_direction = m_direction.Rotated(preRandomBool);
				return true;
			}
			if (GetTerrainFor(map, m_pos.actualPos, m_direction.Rotated(not preRandomBool)) == TerrainKind::Pathway)
			{
				m_direction = m_direction.Rotated(not preRandomBool);
				return true;
			}
		}

		// 通路を曲がるかチェック
		if (proceededCount > 0 &&
			currentTerrain == TerrainKind::Pathway &&
			Random(0, GetTomlParameter<int>(U"play.en_slime_cat.pathway_turn_unlikely")) == 0)
		{
			if (CanMoveTo(map, m_pos.actualPos, m_direction.Rotated(preRandomBool)))
			{
				m_direction = m_direction.Rotated(preRandomBool);
				return true;
			}
			if (CanMoveTo(map, m_pos.actualPos, m_direction.Rotated(not preRandomBool)))
			{
				m_direction = m_direction.Rotated(not preRandomBool);
				return true;
			}
		}
		return false;
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
