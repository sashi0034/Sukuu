#include "stdafx.h"
#include "EndingBackground.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Play/Forward.h"
#include "Play/Chara/CharaUtil.h"
#include "Play/Player_detail/PlayerAnimation.h"

namespace
{
	using namespace Ending;

	constexpr Size mapSize{48, 20};
	constexpr int maxPlantSize = 48;

	enum class PlantKind
	{
		NoPlant,
		Bush1,
		Bush2,
		TreeS,
		TreeL,
		Stone1,
		Stone2,
		Moss1,
		Moss2,
	};

	struct BgPlant
	{
		PlantKind kind = PlantKind::NoPlant;
	};

	constexpr Point getPlantPadding(const Size& size)
	{
		return Point{(Px_16 - size.x) / 2, Px_16 - size.y};
	}

	constexpr Point getPlantPadding(int size)
	{
		return getPlantPadding(Size{size, size});
	}
}

struct Ending::EndingBackground::Impl
{
	Play::AnimTimer m_animTimer{};
	Vec2 m_playerPos{};

	Grid<BgPlant> m_plants{};

	void Init()
	{
		m_playerPos = {-32.0, mapSize.y * Px_16 / 2 - Play::CellPx_24};
		m_plants.resize(mapSize);

		addPlant(PlantKind::Bush1, 32);
		addPlant(PlantKind::Bush2, 32);
		addPlant(PlantKind::TreeL, 24);
		addPlant(PlantKind::TreeS, 24);
		addPlant(PlantKind::Stone1, 24);
		addPlant(PlantKind::Stone2, 24);
		addPlant(PlantKind::Moss1, 24);
		addPlant(PlantKind::Moss2, 24);
	}

	void Update()
	{
		m_animTimer.Tick();
		const ScopedRenderStates2D state{SamplerState::BorderNearest};

		for (const auto p : step({}, mapSize / 4, {4, 4}))
		{
			(void)TextureAsset(AssetImages::grass_tile_64x64).draw(Vec2{p * Px_16});
		}

		drawPlants();

		m_playerPos.x += 6.0 * Scene::DeltaTime();
		(void)Play::GetUsualPlayerTexture(Dir4::Right, m_animTimer, true).draw(m_playerPos);
	}

private:
	void addPlant(PlantKind kind, int n)
	{
		for (int i = 0; i < n; ++i)
		{
			Point pos{};
			while (true)
			{
				pos = RandomPoint(Rect(mapSize));
				if (m_plants[pos].kind != PlantKind::NoPlant) continue;;
				break;
			}
			m_plants[pos] = {
				.kind = kind,
			};
		}
	}

	void drawPlants()
	{
		const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint() / Px_16;
		const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint() / Px_16;
		constexpr int subSpace = (maxPlantSize / Px_16) - 1;

		for (int y = mapTl.y - 1; y < mapBr.y + 1 + subSpace; ++y)
		{
			for (int x = mapTl.x - 1; x < mapBr.x + subSpace; ++x)
			{
				auto point = Point{x, y};
				if (m_plants.inBounds(point) == false) continue;
				drawPlantOf(m_plants[point], point * Px_16);
			}
		}
	}

	void drawPlantOf(const BgPlant& p, const Vec2& pos)
	{
		switch (p.kind)
		{
		case PlantKind::Bush1:
			TextureAsset(AssetImages::bush_16x16)(m_animTimer.SliceFrames(500, 2) * 16, 0, Size::One() * 16)
				.draw(pos + getPlantPadding(16));
			break;
		case PlantKind::Bush2:
			TextureAsset(AssetImages::bush_16x16)(m_animTimer.SliceFrames(500, 2) * 16, 16, Size::One() * 16)
				.draw(pos + getPlantPadding(16));
			break;
		case PlantKind::TreeS:
			TextureAsset(AssetImages::tree_16x16)(m_animTimer.SliceFrames(300, 4) * 16, 0, Size::One() * 16)
				.draw(pos + getPlantPadding(16));
			break;
		case PlantKind::TreeL:
			TextureAsset(AssetImages::tree_48x48)(m_animTimer.SliceFrames(200, 6) * 48, 0, Size::One() * 48)
				.draw(pos + getPlantPadding(48));
			break;
		case PlantKind::Stone1:
			TextureAsset(AssetImages::mixed_nature_16x16)(Point(0, 0) * 16, Size::One() * 16)
				.draw(pos + getPlantPadding(16));
			break;
		case PlantKind::Stone2:
			TextureAsset(AssetImages::mixed_nature_16x16)(Point(1, 0) * 16, Size::One() * 16)
				.draw(pos + getPlantPadding(16));
			break;
		case PlantKind::Moss1:
			TextureAsset(AssetImages::mixed_nature_16x16)(Point(0, 1) * 16, Size::One() * 16)
				.draw(pos + getPlantPadding(16));
			break;
			break;
		case PlantKind::Moss2:
			TextureAsset(AssetImages::mixed_nature_16x16)(Point(1, 1) * 16, Size::One() * 16)
				.draw(pos + getPlantPadding(16));
			break;
		default: ;
		}
	}
};

namespace Ending
{
	EndingBackground::EndingBackground() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EndingBackground::Init()
	{
		p_impl->Init();
	}

	void EndingBackground::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	Vec2 EndingBackground::PlainSize() const
	{
		return mapSize * Px_16;
	}
}
