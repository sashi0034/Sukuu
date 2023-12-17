#include "stdafx.h"
#include "EnHandMaster.h"

#include "Assets.generated.h"
#include "EnemyUtil.h"
#include "Play/PlayCore.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.en_hand_master." + key);
	}

	constexpr Rect spriteRect{0, 0, 32, 32};

	using namespace Play;

	Vec2 followingPlayerPos()
	{
		constexpr Vec2 offset{0, 0};
		return PlayCore::Instance().GetPlayer().GetActualViewPos() + offset;
	}
}

struct EnHandMaster::Impl : EnemyTransform
{
	bool m_mimicking{true};
	double m_mimicTransition{};
	bool m_capturedPlayer{};
	ActorWeak m_flowchart{};
	ActorWeak m_captureStartMotion{};


	void Update(ActorView self)
	{
		m_animTimer.Tick();

		// プレイヤーとの当たり判定
		if (CheckSendEnemyCollide(PlayCore::Instance().GetPlayer(), *this, EnemyKind::HandMaster))
		{
			// 捕捉開始
			m_flowchart.Kill();
			m_capturedPlayer = true;
			m_captureStartMotion = AnimateEasing<EaseOutBack, EaseOption::Default | EaseOption::IgnoreTimeScale>(
				self, &m_pos.viewPos, followingPlayerPos(), 0.3);
		}

		// プレイヤー補足
		if (m_capturedPlayer && m_captureStartMotion.IsDead())
		{
			m_pos.viewPos = followingPlayerPos();
		}

		const AssetNameView emotion = [&]()
		{
			if (m_capturedPlayer) return U"🤗";
			if (m_trapped == EnemyTrappedState::Captured) return U"😬";
			if (m_playerTracker.IsTracking()) return U"😎";
			return U"";
		}();

		if (m_mimicking)
		{
			// 擬態中
			drawMimic();
			if (not emotion.empty()) DrawCharaEmotion(GetDrawPos(), emotion);
		}
		else
		{
			DrawEnemyBasically(*this, emotion);
		}
	}

	Vec2 GetDrawPos() const override
	{
		return m_pos.viewPos.movedBy(m_animOffset + GetCharacterCellPadding(spriteRect.size));
	}

	void StartFlowchart(ActorBase& self)
	{
		m_flowchart = StartCoro(self, [this, self](YieldExtended yield)
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
			TextureAsset(AssetImages::otete_32x32)(
				spriteRect.movedBy(m_animTimer.SliceFrames(150, 4) * spriteRect.w, 0));
		return m_dir == Dir4::Right || m_dir == Dir4::Down ? t : t.mirrored();
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		yield();

		// 階段に擬態
		if (m_mimicking)
		{
			mimicFloor(yield, self);
			m_mimicking = false;
		}

		// 移動中
		const bool leftPriority = RandomBool(0.5);
		while (true)
		{
			auto&& map = PlayCore::Instance().GetMap();
			auto&& gimmick = PlayCore::Instance().GetGimmick();
			const TerrainKind currentTerrain = GetTerrainAt(map, m_pos.actualPos);
			const auto currentPoint = m_pos.actualPos.MapPoint();

			// ギミック確認
			CheckEnemyTrappingGimmick(yield, self, currentPoint, *this);

			// プレイヤー追跡チェック
			m_playerTracker.Track(
				m_dir,
				currentPoint,
				getToml<int>(U"max_player_concern"),
				[&]() { return; });

			// 曲がるかチェック
			if (not m_playerTracker.IsTracking())
				checkTurn(map, gimmick, currentTerrain, leftPriority);

			// 進行可能方向に向く
			if (FaceEnemyMovableDir(m_dir, m_pos, map, gimmick, leftPriority) == false) break;

			// 移動開始
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			const double moveDuration = m_playerTracker.IsTracking()
				                            ? getToml<double>(U"tracking_duration")
				                            : getToml<double>(U"move_duration");;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration);
		}
	}

	void drawMimic() const
	{
		if (m_mimicTransition == 0)
		{
			// 階段描画
			(void)TextureAsset(AssetImages::stairs_24x24).draw(m_pos.viewPos);
		}
		else
		{
			const auto center = m_pos.viewPos.movedBy(Vec2::One() * CellPx_24 / 2);
			(void)TextureAsset(AssetImages::stairs_24x24)
			      .scaled(1 - OverextendEaseInBack<3>(m_mimicTransition))
			      .drawAt(center);
			(void)GetTexture()
			      .scaled(OverextendEaseOutBack<3>(m_mimicTransition))
			      .drawAt(GetDrawPos().movedBy(spriteRect.size / 2));
		}
	}

	void mimicFloor(YieldExtended& yield, ActorView self)
	{
		const auto selfPoint = m_pos.actualPos.MapPoint();
		while (true)
		{
			yield();
			const bool nearPlayer =
				PlayCore::Instance().GetPlayer().DistField()[selfPoint].distance <= 2;
			// プレイヤーが近くに来た
			if (nearPlayer) break;
		}

		m_dir = Dir4::FromXY(PlayCore::Instance().GetPlayer().CurrentPos().actualPos - m_pos.actualPos);

		// 起動
		yield.WaitForExpire(AnimateEasing<EaseInLinear>(self, &m_mimicTransition, 1.0, 0.5));
		// yield.WaitForTime(0.5);
	}

	bool checkTurn(const MapGrid& map, const GimmickGrid& gimmick, TerrainKind currentTerrain, bool leftPriority)
	{
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
	EnHandMaster::EnHandMaster() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EnHandMaster::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayCore::Instance().GetMap()));
		p_impl->StartFlowchart(*this);
	}

	void EnHandMaster::Update()
	{
		ActorBase::Update();
		p_impl->Update(*this);
		if (p_impl->m_trapped == EnemyTrappedState::Killed) Kill();
	}

	double EnHandMaster::OrderPriority() const
	{
		return p_impl->m_mimicking
			       ? BgEffectPriority + 1
			       : (p_impl->m_capturedPlayer
				          ? PlayCore::Instance().GetPlayer().OrderPriority() + 1
				          : CharaOrderPriority(p_impl->m_pos));
	}

	bool EnHandMaster::SendDamageCollider(const ItemAttackerAffair& attacker, const RectF& collider)
	{
		if (not IsEnemyCollided(*p_impl, collider)) return false;
		PerformEnemyDestroyed(attacker, *p_impl);
		Kill();
		return true;
	}

	const CharaPosition& EnHandMaster::Pos() const
	{
		return p_impl->m_pos;
	}
}
