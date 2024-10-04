#include "stdafx.h"
#include "EndingBackground.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "Play/Forward.h"
#include "Play/Chara/CharaUtil.h"
#include "Play/Player_detail/PlayerAnimation.h"

namespace
{
	using namespace Ending;

	constexpr Size mapSize{75, 25};
	constexpr int maxPlantSize = 48;

	// 15x25
	Array<String> mapStringPattern()
	{
		return {
			U"---------------",
			U"---------------",
			U"---------------",
			U"FFFFFFFFFFFFFFF",
			U"FFFFFFFFFFFFFFF",
			U"               ",
			U" T   T   T   T ",
			U"   T       T   ",
			U" T   T   T   T ",
			U"  U U     U U  ",
			U" F F F F F F F ",
			U"FFFFFFFFFFFFFFF",
			U"---------------",
			U"FFFFFFFFFFFFFFF",
			U" F F F F F F F ",
			U"  U U     U U  ",
			U"   T  FFF  T   ",
			U" T   FFFFF   T ",
			U"   T  FFF  T   ",
			U"               ",
			U"FFFFFFFFFFFFFFF",
			U"FFFFFFFFFFFFFFF",
			U"---------------",
			U"---------------",
			U"---------------",
		};
	}

	struct PlantLocation
	{
		Vec2 pos;
		int hash;
	};

	struct VegetationData
	{
		Array<PlantLocation> flowers;
		Array<PlantLocation> bigTrees;
		Array<PlantLocation> smallTrees;
		Array<PlantLocation> mixed;
	};

	constexpr Point getPlantPadding(const Size& size)
	{
		return Point{(Px_16 - size.x) / 2, Px_16 - size.y};
	}

	constexpr Point getPlantPadding(int size)
	{
		return getPlantPadding(Size{size, size});
	}

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"ending.background." + key);
	}
}

struct Ending::EndingBackground::Impl
{
	RenderTexture m_rt{};

	Play::AnimTimer m_animTimer{};
	Vec2 m_playerPos{};
	double m_cameraX{};

	VegetationData m_vegetation{};

	void Init()
	{
		// m_rgbShifter = HLSL{U"asset/shader/rgb_shift.hlsl", U"PS"};
		m_rt = RenderTexture(Scene::Size());

		m_playerPos = {-32.0, mapSize.y * Px_16 / 2 - Play::CellPx_24};

		// 植生データ
		const auto mapData = mapStringPattern();
		for (int y = 0; y < mapSize.y; ++y)
		{
			for (int x = 0; x < mapSize.x; ++x)
			{
				const auto c = mapData[y][x % mapData[0].size()];
				const auto pos = Vec2{x, y} * Px_16;
				const auto location = PlantLocation{pos, x + y};
				switch (c)
				{
				case U'F':
					m_vegetation.flowers.push_back(location);
					break;
				case U'T':
					m_vegetation.bigTrees.push_back(location);
					break;
				case U'U':
					m_vegetation.smallTrees.push_back(location);
					break;
				case U' ':
					m_vegetation.mixed.push_back(location);
					break;
				default: ;
				}
			}
		}
	}

	void Update()
	{
		m_animTimer.Tick();

		updateBg();

		// 手前のエフェクト
		[&]
		{
			const int h = getToml<int>(U"grad_height") + Periodic::Sine1_1(4.0) * 20.0;
			const Color c = getToml<Color>(U"grad_color");
			Rect(0, 0, Scene::Size().x, h)
				.draw(Arg::top = ColorF{c, 0.7}, Arg::bottom = ColorF{c, 0.0});
			Rect(0, Scene::Size().y - h, Scene::Size().x, h)
				.draw(Arg::top = ColorF{c, 0.0}, Arg::bottom = ColorF{c, 0.7});
		}();
	}

private:
	void updateBg()
	{
		m_cameraX += Scene::DeltaTime() * 4.0;

		m_playerPos.x += 6.0 * Scene::DeltaTime();

		constexpr int mapCenterY = (mapSize.y * Px_16 / 2);
		const auto cameraPos = Vec2{m_cameraX, mapCenterY};
		const double cameraScale = 4.5 + 0.5 * Periodic::Sine1_1(12.0s);
		const auto center = Vec2{0, Scene::Center().y};
		const Transformer2D transformer0{
			Mat3x2::Translate(center).translated(-cameraPos).scaled(cameraScale, center)
		};

		drawBg();
	}

	void drawBg() const
	{
		const ScopedRenderStates2D state{SamplerState::ClampNearest};

		// 原っぱのプレーン
		{
			Rect(Scene::Size()).draw(ColorF{0.38, 0.6, 0});

			for (const auto p : step({}, mapSize / 4, {4, 4}))
			{
				(void)TextureAsset(AssetImages::grass_tile_64x64).draw(Vec2{p * Px_16});
			}
		}

		const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		const auto mapTl = inversed.transformPoint(Vec2{0, 0});
		const auto mapBr = inversed.transformPoint(Scene::Size());
		constexpr int subSpace = (maxPlantSize / Px_16) - 1;

		// 花描画
		{
			for (const auto& l : m_vegetation.flowers)
			{
				if (not InRange<double>(l.pos.x, mapTl.x - 16, mapBr.x)) continue;

				const int v = 16 * (l.hash % 3);
				TextureAsset(AssetImages::flowers_16x16)(
						m_animTimer.SliceFramesWithOffset(200, 4, l.hash) * 16, v, Size::One() * 16)
					.draw(l.pos + getPlantPadding(16));
			}
		}

		// プレイヤー描画
		(void)GetUsualPlayerTexture(Dir4::Right, m_animTimer, true)
			.draw(m_playerPos.movedBy(0, -Play::CharacterPadding_4));

		// いろいろ描画
		{
			for (const auto& l : m_vegetation.mixed)
			{
				if (not InRange<double>(l.pos.x, mapTl.x - 16, mapBr.x)) continue;

				const int v = 16 * (l.hash % 6);
				switch (l.hash % 6)
				{
				case 0:
					TextureAsset(AssetImages::mixed_nature_16x16)(0, 0, Size::One() * 16).draw(
						l.pos + getPlantPadding(16));
					break;
				case 1:
					TextureAsset(AssetImages::mixed_nature_16x16)(Point{0, 1} * 16, Size::One() * 16).draw(
						l.pos + getPlantPadding(16));
					break;
				case 2:
					TextureAsset(AssetImages::bush_16x16)(
							Point{m_animTimer.SliceFramesWithOffset(250, 2, l.hash), 0} * 16, Size::One() * 16)
						.draw(l.pos + getPlantPadding(16));
					break;
				case 3:
					TextureAsset(AssetImages::mixed_nature_16x16)(1, 0, Size::One() * 16).draw(
						l.pos + getPlantPadding(16));
					break;
				case 4:
					TextureAsset(AssetImages::mixed_nature_16x16)(1, 1, Size::One() * 16).draw(
						l.pos + getPlantPadding(16));
					break;
				case 5:
					TextureAsset(AssetImages::bush_16x16)(
							Point{m_animTimer.SliceFramesWithOffset(250, 2, l.hash), 1} * 16, Size::One() * 16)
						.draw(l.pos + getPlantPadding(16));
					break;
				default:
					break;
				}
			}
		}

		// 小さい木描画
		{
			for (const auto& l : m_vegetation.smallTrees)
			{
				if (not InRange<double>(l.pos.x, mapTl.x - 16, mapBr.x)) continue;

				TextureAsset(AssetImages::tree_16x16)(
						m_animTimer.SliceFramesWithOffset(150, 4, l.hash) * 16, 0, Size::One() * 16)
					.draw(l.pos + getPlantPadding(16));
			}
		}

		// 大きい木描画
		{
			for (const auto& l : m_vegetation.bigTrees)
			{
				if (not InRange<double>(l.pos.x, mapTl.x - 48, mapBr.x)) continue;

				TextureAsset(AssetImages::tree_48x48)(
						m_animTimer.SliceFramesWithOffset(200, 6, l.hash) * 48, 0, Size::One() * 48)
					.draw(l.pos + getPlantPadding(48));
			}
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
