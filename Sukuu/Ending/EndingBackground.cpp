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

	constexpr Size mapSize{64, 20};
	constexpr int maxPlantSize = 48;

	struct RgbShiftCb
	{
		float amount{};
	};

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

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"ending.background." + key);
	}
}

struct Ending::EndingBackground::Impl
{
	RenderTexture m_rt{};
	// PixelShader m_rgbShifter{};
	ConstantBuffer<RgbShiftCb> m_cb{};

	Play::AnimTimer m_animTimer{};
	Vec2 m_playerPos{};
	double m_cameraX{};

	Grid<BgPlant> m_plants{};

	void Init()
	{
		// m_rgbShifter = HLSL{U"asset/shader/rgb_shift.hlsl", U"PS"};
		m_rt = RenderTexture(Scene::Size());

		m_playerPos = {-32.0, mapSize.y * Px_16 / 2 - Play::CellPx_24};
		m_plants.resize(mapSize);

		addPlant(PlantKind::Bush1, 96, 1, {
			         {1, 0, 1, 0},
			         {0, 1, 0, 0},
			         {1, 0, 1, 0},
			         {0, 0, 0, 0}
		         });
		addPlant(PlantKind::Bush2, 96, 1, {
			         {0, 1, 0, 0},
			         {1, 0, 1, 0},
			         {0, 1, 0, 0},
			         {0, 0, 0, 0}
		         });
		addPlant(PlantKind::TreeL, 64, 1, {
			         {0, 0, 0, 0},
			         {0, 1, 0, 0},
			         {0, 0, 0, 1},
			         {0, 1, 0, 0}
		         });
		addPlant(PlantKind::TreeS, 64, 2, {
			         {0, 0, 0, 1},
			         {0, 0, 1, 0},
			         {0, 1, 0, 1},
			         {1, 0, 1, 0}
		         });
		addPlant(PlantKind::Stone1, 64, 0, {
			         {1, 0, 1, 0},
			         {0, 1, 0, 1},
			         {1, 0, 1, 0},
			         {0, 1, 0, 1},
		         });
		addPlant(PlantKind::Stone2, 64, 0, {
			         {0, 1, 0, 1},
			         {1, 0, 1, 0},
			         {0, 1, 0, 1},
			         {1, 0, 1, 0},
		         });
		addPlant(PlantKind::Moss1, 96, 0, {
			         {1, 0, 1, 0},
			         {0, 1, 0, 1},
			         {1, 0, 1, 0},
			         {0, 1, 0, 1},
		         });
		addPlant(PlantKind::Moss2, 96, 0, {
			         {0, 1, 0, 1},
			         {1, 0, 1, 0},
			         {0, 1, 0, 1},
			         {1, 0, 1, 0},
		         });
	}

	void Update()
	{
		m_animTimer.Tick();

#if 0
		// 色収差なんか微妙なので没
		[&]
		{
			const ScopedRenderTarget2D target{m_rt};
			updateBg();
		};

		[&]
		{
			m_cb->amount = 0.005 * Periodic::Sine0_1(12.0s);
			const ScopedCustomShader2D shader{m_rgbShifter};
			Graphics2D::SetPSConstantBuffer(1, m_cb);
			(void)m_rt.draw();
		}();
#endif
		updateBg();

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
	void addPlant(PlantKind kind, int n, int duDy, const Grid<uint8>& pattern)
	{
		const int pw = (m_plants.size().x / pattern.size().x) * pattern.size().x;
		const auto patternOk = [&](const Point& p) -> bool
		{
			const int y1 = p.y / pattern.size().y;
			const int u = y1 * duDy;
			const auto p1 = Point((p.x - u + pw) % pattern.size().x, (p.y) % pattern.size().y);
			return pattern[p1] != 0;
		};

		for (size_t i = 0; i < n; ++i)
		{
			for (const auto s : step(Constants::BigValue_100000))
			{
				Point pos = RandomPoint(Rect(m_plants.size()));
				if (m_plants[pos].kind != PlantKind::NoPlant) continue;
				if (not patternOk(pos)) continue;
				m_plants[pos] = {
					.kind = kind,
				};
				break;
			}
		}
	}

	void updateBg()
	{
		m_cameraX += Scene::DeltaTime() * 4.0;

		m_playerPos.x += 6.0 * Scene::DeltaTime();

		constexpr int mapCenterY = (mapSize.y * Px_16 / 2) - Px_16 / 2;
		const auto cameraPos = Vec2{m_cameraX, mapCenterY};
		const double cameraScale = 4.5 + 0.5 * Periodic::Sine1_1(12.0s);
		const auto center = Vec2{0, Scene::Center().y};
		const Transformer2D transformer0{
			Mat3x2::Translate(center).translated(-cameraPos).scaled(cameraScale, center)
		};

		const ScopedRenderStates2D state{SamplerState::BorderNearest};

		for (const auto p : step({}, mapSize / 4, {4, 4}))
		{
			(void)TextureAsset(AssetImages::grass_tile_64x64).draw(Vec2{p * Px_16});
		}

		drawPlants();
	}

	void drawPlants()
	{
		const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint() / Px_16;
		const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint() / Px_16;
		constexpr int subSpace = (maxPlantSize / Px_16) - 1;

		bool renderedPlayer{};
		for (int y = mapTl.y - 1; y < mapBr.y + 1 + subSpace; ++y)
		{
			bool playerLine{};
			if (not renderedPlayer && (y - 1) * Px_16 > m_playerPos.y)
			{
				// プレイヤー描画
				(void)Play::GetUsualPlayerTexture(Dir4::Right, m_animTimer, true).draw(m_playerPos);
				renderedPlayer = true;
				playerLine = true;
			}
			for (int x = mapTl.x - 1; x < mapBr.x + subSpace; ++x)
			{
				auto point = Point{x, y};
				if (m_plants.inBounds(point) == false) continue;
				if (playerLine && m_plants[point].kind == PlantKind::Moss2) continue;
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
