#include "stdafx.h"
#include "TitleBackground.h"

#include "Play/Map/BgMapDrawer.h"
#include "Play/Player_detail/PlayerAnimation.h"

namespace
{
	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"title.background." + key);
	}

	constexpr Size maxMapSize{55, 55};
	constexpr std::array<StringView, 15> mapData = {
		U"               ", // 00
		U"               ", // 01
		U"      ---      ", // 02
		U"      -#-      ", // 03
		U"      ---      ", // 04
		U"     -----     ", // 05
		U"  -----------  ", // 06
		U"  -----------  ", // 07
		U"  -----------  ", // 08
		U"     -----     ", // 09
		U"      ---      ", // 0a
		U"      ---      ", // 0b
		U"      ---      ", // 0c
		U"               ", // 0d
		U"               ", // 0e
	};
}

struct Title::TitleBackground::Impl
{
	RenderTexture m_mapTexture{};
	MSRenderTexture m_renderTexture{Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes};
	SimpleFollowCamera3D m_camera{};
	double m_cameraFollowDeg{};
	Mesh m_playerBillboard{MeshData::Billboard()};
	Play::AnimTimer m_animTimer{};

	void Init()
	{
		constexpr int mapW = mapData[0].size();
		constexpr int mapH = mapData.size();
		constexpr auto mapSize = Size(mapW, mapH);

		m_mapTexture = RenderTexture(maxMapSize * Play::CellPx_24);
		Play::MapGrid mapGrid{maxMapSize};
		for (const auto p : step(maxMapSize))
			mapGrid.At(p).kind = Play::TerrainKind::Wall;
		Point stairsPoint{};
		for (const auto p : step(mapSize))
		{
			auto targetP = p.movedBy((maxMapSize - mapSize) / 2);
			switch (mapData[p.y][p.x])
			{
			case U'-':
				mapGrid.At(targetP).kind = Play::TerrainKind::Floor;
				break;
			case U'#':
				stairsPoint = targetP;
				mapGrid.At(targetP).kind = Play::TerrainKind::Floor;
				break;
			default:
				mapGrid.At(targetP).kind = Play::TerrainKind::Wall;
				break;
			}
		}

		ScopedRenderTarget2D target{m_mapTexture};
		for (const auto p : step(maxMapSize))
		{
			Play::DrawBgMapTileAt(mapGrid, p.x, p.y);
		}
		// 階段
		(void)TextureAsset(AssetImages::stairs_24x24)({0, 0}, Size{1, 1} * Play::CellPx_24)
			.draw(stairsPoint * Play::CellPx_24);

		resetCamera();
	}

	void Update()
	{
#if _DEBUG
		if (KeyC.down())
		{
			resetCamera();
		}
#endif

		m_cameraFollowDeg += GetDeltaTime() * getToml<double>(U"camera_rotation_speed");
		m_camera.setTarget(getToml<Vec3>(U"camera_target_position"), ToRadians(m_cameraFollowDeg));
		m_camera.setFollowOffset(getToml<double>(U"camera_follow_distance"), getToml<double>(U"camera_follow_height"));
		m_camera.update(2.0, GetDeltaTime());

		m_animTimer.Tick();

		// 3D描画
		draw3D();

		// RenderTextureを2Dシーンに描画
		apply3D();
	}

private:
	void resetCamera()
	{
		m_camera = SimpleFollowCamera3D(m_renderTexture.size(),
		                                ToRadians(getToml<double>(U"camera_fov")),
		                                getToml<Vec3>(U"camera_initial_position"),
		                                0.0_deg,
		                                16.0,
		                                8.0);
	}

	void draw3D() const
	{
		Graphics3D::SetCameraTransform(m_camera);
		auto sunDir = m_camera.getEyePosition().normalized();
		sunDir.y = 0.5;
		Graphics3D::SetSunDirection(sunDir);

		const ScopedRenderTarget3D target{m_renderTexture.clear(getToml<ColorF>(U"bg_color"))};
		const ScopedRenderStates3D state{SamplerState::BorderNearest};

		constexpr double dotScale = 2.0;

		(void)Plane{maxMapSize.x * dotScale}.draw(m_mapTexture, Palette::Thistle);

		const ScopedRenderStates3D states{BlendState::Default2D, DepthStencilState::DepthTest};

		m_playerBillboard.draw(
			m_camera.billboard(Vec3{0, 1, 0}, dotScale),
			Play::GetUsualPlayerTexture(Dir4::FromXY({-m_camera.getEyePosition().xz()}), m_animTimer, false),
			Palette::Thistle);
	}

	void apply3D() const
	{
		Graphics3D::Flush();
		m_renderTexture.resolve();
		(void)m_renderTexture.resized(Scene::Size()).draw();
	}
};

namespace Title
{
	TitleBackground::TitleBackground() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TitleBackground::Init()
	{
		p_impl->Init();
	}

	void TitleBackground::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}
}
