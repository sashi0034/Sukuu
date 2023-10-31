#include "stdafx.h"
#include "EnCatfish.h"

#include "Assets.generated.h"
#include "EnemyUtil.h"
#include "Play/PlayScene.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.en_catfish." + key);
	}

	constexpr Rect spriteRect{0, 0, 24, 24};
}

struct Play::EnCatfish::Impl : EnemyTransform
{
	bool m_doingLostPenalty = false;

	void Update()
	{
		// プレイヤーとの当たり判定
		CheckSendEnemyCollide(PlayScene::Instance().GetPlayer(), *this, EnemyKind::Catfish);

		// アニメーション更新
		m_animTimer.Tick();
		const auto drawingPos = GetDrawPos();
		(void)GetTexture().draw(drawingPos);

		// 吹き出し描画
		const AssetNameView emotion = [&]()
		{
			if (m_trapped == EnemyTrappedState::Captured) return U"😬";
			if (m_playerTracker.IsTracking()) return U"😎";
			if (m_doingLostPenalty) return U"🤔";
			return U"";
		}();

		if (not emotion.empty()) DrawCharaEmotion(drawingPos, emotion);
	}

	Vec2 GetDrawPos() const
	{
		return m_pos.viewPos.movedBy(m_animOffset + GetCharacterCellPadding(spriteRect.size));
	}

	void StartFlowchart(ActorBase& self)
	{
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			while (true)
			{
				flowchartLoop(yield, self);
			}
		});
	}

	TextureRegion GetTexture() const override
	{
		auto&& t =
			TextureAsset(AssetImages::catfish_24x24)(
				spriteRect.movedBy(m_animTimer.SliceFrames(150, 5) * spriteRect.w, 0));
		return m_dir == Dir4::Left || m_dir == Dir4::Up ? t : t.mirrored();
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		yield();

		// 移動中
		int proceededCount{};
		while (true)
		{
			auto&& map = PlayScene::Instance().GetMap();
			auto&& gimmick = PlayScene::Instance().GetGimmick();
			const TerrainKind currentTerrain = GetTerrainAt(map, m_pos.actualPos);
			const auto currentPoint = m_pos.actualPos.MapPoint();

			// ギミック確認
			CheckEnemyTrappingGimmick(yield, self, currentPoint, *this);

			// プレイヤー追跡チェック
			m_playerTracker.Track(
				m_dir,
				currentPoint,
				getToml<int>(U"max_player_concern"),
				[&]()
				{
					m_doingLostPenalty = true;
					yield.WaitForTime(getToml<double>(U"lost_player_penalty"));
					m_doingLostPenalty = false;
					proceededCount = 0;
				});

			// 曲がるかチェック
			if (not m_playerTracker.IsTracking())
				checkTurn(map, gimmick, currentTerrain, RandomBool(0.5));

			// 進行可能方向に向く
			if (FaceEnemyMovableDir(m_dir, m_pos, map, gimmick, RandomBool(0.5)) == false) break;

			// 移動
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			const double moveDuration = getToml<double>(U"move_duration");
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration);
			proceededCount++;
		}
	}

	bool checkTurn(const MapGrid& map, const GimmickGrid& gimmick, TerrainKind currentTerrain, bool leftPriority)
	{
		// 頭が悪いので、特に関係なく曲がる
		if (RandomBool(getToml<double>(U"random_corner")))
		{
			m_dir = Random(0, 3);
			return true;
		}

		// 部屋から出れるかチェック
		const double cornerPr = getToml<double>(U"corner_pr");
		if (currentTerrain == TerrainKind::Floor &&
			RandomBool(cornerPr))
		{
			return RotateEnemyDirFacingPathway(m_dir, m_pos, map, leftPriority);
		}

		// 通路を曲がるかチェック
		if (currentTerrain == TerrainKind::Pathway &&
			RandomBool(cornerPr))
		{
			return RotateEnemyDirFacingMovable(m_dir, m_pos, map, gimmick, leftPriority);
		}
		return false;
	}
};

namespace Play
{
	EnCatfish::EnCatfish() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EnCatfish::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayScene::Instance().GetMap()));
		p_impl->StartFlowchart(*this);
	}

	void EnCatfish::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		if (p_impl->m_trapped == EnemyTrappedState::Killed) Kill();
	}

	double EnCatfish::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}

	bool EnCatfish::SendDamageCollider(const ItemAttackerAffair& attacker, const RectF& collider)
	{
		if (not IsEnemyCollided(*p_impl, collider)) return false;
		PerformEnemyDestroyed(attacker, *p_impl);
		Kill();
		return true;
	}

	const CharaPosition& EnCatfish::Pos() const
	{
		return p_impl->m_pos;
	}
}
