#include "stdafx.h"
#include "CharaUtil.h"

#include "Play/PlayCore.h"
#include "Play/Effect/DamageCounterEffect.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace Play
{
	int AnimTimer::SliceFrames(uint64 milliInterval, const Array<int>& pattern) const
	{
		const uint64 t = m_timer * 1000;
		const int index = (t % (milliInterval * pattern.size())) / milliInterval;
		return pattern[index];
	}

	int AnimTimer::SliceFrames(uint64 milliInterval, uint32 numFrames) const
	{
		const uint64 t = m_timer * 1000;
		return (t % (milliInterval * numFrames)) / milliInterval;
	}

	TerrainKind GetTerrainAt(const MapGrid& map, const CharaVec2& pos)
	{
		const Point p = pos.MapPoint();
		if (map.Data().inBounds(p) == false) return TerrainKind::None;
		return map.Data()[p].kind;
	}

	TerrainKind GetTerrainFor(const MapGrid& map, const CharaVec2& pos, Dir4Type dir)
	{
		const Point p = pos.MapPoint() + dir.ToXY().asPoint();
		if (map.Data().inBounds(p) == false) return TerrainKind::None;
		return map.Data()[p].kind;
	}

	bool canMoveAtInternal(const MapGrid& map, const Point next)
	{
		if (map.Data().inBounds(next) == false) return false;
		return map.At(next).kind != TerrainKind::Wall;
	}

	bool CanMoveTo(const MapGrid& map, const CharaVec2& currentActualPos, Dir4Type dir)
	{
		const Point next = currentActualPos.MapPoint() + dir.ToXY().asPoint();
		return canMoveAtInternal(map, next);
	}

	bool CanMovePointTo(const MapGrid& map, const Point& point, Dir4Type dir)
	{
		const Point next = point + dir.ToXY().asPoint();
		return canMoveAtInternal(map, next);
	}

	bool CanMovePointAt(const MapGrid& map, const Point& point)
	{
		return canMoveAtInternal(map, point);
	}

	Point GetArrowWarpPoint(const MapGrid& map, const GimmickGrid& gimmickGrid, const Point& point)
	{
		const auto dir = GimmickArrowToDir(gimmickGrid[point]).ToXY().asPoint();
		if (dir == Point{0, 0}) return point;
		auto checking = point + dir;
		while (true)
		{
			checking += dir;
			if (checking.x < 0) checking.x = gimmickGrid.size().x - 2;
			if (checking.y < 0) checking.y = gimmickGrid.size().y - 2;
			if (checking.x >= gimmickGrid.size().x) checking.x = 1;
			if (checking.y >= gimmickGrid.size().y) checking.y = 1;
			if (map.At(checking).kind == TerrainKind::Wall) continue;
			return checking;
		}
	}

	template <double easing(double)>
	void ProcessMoveCharaPos(
		YieldExtended& yield, ActorView self, CharaPosition& pos, const Vec2& nextPos, double moveDuration)
	{
		// 実座標を移動
		Util::AnimateEasing<easing, EaseOption::Default>(
			self, &pos.actualPos, CharaVec2(nextPos),
			moveDuration);
		// ビュー座標を移動し、これを待機
		yield.WaitForExpire(
			Util::AnimateEasing<easing, EaseOption::None>(
				self, &pos.viewPos, nextPos,
				moveDuration)
		);
	}

	void ProcessArrowWarpCharaPos(
		YieldExtended& yield, ActorView self, CharaPosition& pos, Vec2& jumpOffset, const Vec2& nextPos)
	{
		constexpr double moveDuration = 0.5;

		// ジャンプ表現
		Util::AnimateEasing<BoomerangParabola>(self, &jumpOffset, Vec2{0, -48.0}, moveDuration);

		Util::AnimateEasing<EaseInLinear>(
			self, &pos.actualPos, CharaVec2(nextPos), moveDuration);
		// 通常移動とは違い、微小時間の慣性を無視
		yield.WaitForExpire(
			Util::AnimateEasing<EaseInLinear>(self, &pos.viewPos, nextPos, moveDuration)
		);
	}

	Vec2 GetInitialPos(const MapGrid& map, bool allowPathway)
	{
		if (not allowPathway)
		{
			return map.Rooms().RandomRoomPoint(false) * CellPx_24;
		}
		else
		{
			while (true)
			{
				const auto p = RandomPoint(Rect(map.Data().size())) / 2 * 2 + Point(1, 1);
				if (map.Data().inBounds(p) && map.At(p).kind != TerrainKind::Wall) return p * CellPx_24;
			}
		}
	}

	double CharaOrderPriority(const CharaPosition& pos)
	{
		return 1000.0 + (pos.viewPos.y / (CellPx_24 * 1024));
	}

	void DrawCharaEmotion(const Vec2& drawingPos, AssetNameView emoji)
	{
		ScopedRenderStates2D sampler{SamplerState::ClampLinear};
		const auto drawingRect = RectF{
			drawingPos.movedBy(0, GetTomlParameter<int>(U"play.chara.balloon_offset_y")), {CellPx_24, CellPx_24}
		};

		const auto tri = Triangle(drawingRect.bl().moveBy(4, -1),
		                          drawingRect.bl().moveBy(8, -1),
		                          drawingPos.movedBy(CellPx_24 / 2, 0));

		(void)drawingRect.rounded(4).draw(Palette::White).drawFrame(0.5, Palette::Darkslategray);

		(void)tri.draw(Palette::White);

		(void)TextureAsset(emoji).resized(drawingRect.stretched(-2).size).drawAt(drawingRect.center());
	}

	static void relayTimeDamage(const CharaPosition& pos, int amount, const Color& c, bool isEnemyDamage = false)
	{
		auto&& timeLimiter = PlayCore::Instance().GetTimeLimiter();
		if (timeLimiter.IsCountEnabled() == false) return;
		if (amount > 0)
		{
			timeLimiter.Heal(amount);
		}
		else
		{
			timeLimiter.Damage(-amount, isEnemyDamage);
		}
		PlayCore::Instance().FgEffect().add(EmitDamageCounterEffect({
			.center = (pos.viewPos + Vec2(1, 1) * CellPx_24 / 2),
			.amount = amount,
			.color = c,
		}));
	}

	void RelayTimeDamageAmount(const CharaPosition& pos, int amount, bool isEnemyDamage)
	{
		relayTimeDamage(pos, -amount, isEnemyDamage ? Palette::Orangered : Palette::Lightgray, isEnemyDamage);
	}

	void RelayTimeHealAmount(const CharaPosition& pos, int amount)
	{
		relayTimeDamage(pos, amount, Palette::Gold);
	}
}
