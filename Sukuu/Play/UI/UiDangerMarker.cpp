#include "stdafx.h"
#include "UiDangerMarker.h"

#include "Play/PlayCore.h"
#include "Util/LivePPAddon.h"

using namespace Play;

namespace
{
	constexpr Size iconSize{48, 48};
}

struct UiDangerMarker::Impl
{
	Texture m_iconTexture{};

	Array<RectF> m_markedEnemies{};

	void Init()
	{
		createIconTexture();
	}

	void Update(ActorView self)
	{
		// m_iconTexture.drawAt(Scene::Center(), Palette::Crimson);

		const auto cameraTransform = PlayCore::Instance().GetPlayer().CameraTransform();
		const auto inversedCameraTransform = cameraTransform.inverse();

		constexpr double sceneMargin = iconSize.x / 2 - 4;
		const auto sceneTL = inversedCameraTransform.transformPoint(Vec2{sceneMargin, sceneMargin});
		const auto sceneBR = inversedCameraTransform.transformPoint(Scene::Size() - Vec2{sceneMargin, sceneMargin});
		const auto sceneRect = RectF::FromPoints(sceneTL, sceneBR);

		const double visibleRange =
			// (Scene::Center()).manhattanLength();
			Scene::Center().x + 64.0;

		const Vec2 playerCenter = cameraTransform.transformPoint(
			PlayCore::Instance().GetPlayer().CurrentPos().actualPos + Vec2{CellPx_24, CellPx_24} * 0.5);

		for (const auto& enemyRect : m_markedEnemies)
		{
			if (sceneRect.intersects(enemyRect))
			{
				// 画面内に収まっている場合は描画しない
				continue;
			}

			auto transformedCenter = cameraTransform.transformPoint(enemyRect.center());
			if ((playerCenter - transformedCenter).manhattanLength() > visibleRange)
			{
				// あまりにも遠い場合は描画しない
				continue;
			}

			transformedCenter.clamp(Scene::Rect().stretched(-sceneMargin));

			(void)m_iconTexture.drawAt(transformedCenter, Palette::Crimson);
		}

		m_markedEnemies.clear();

#ifdef _DEBUG
		if (IsLivePPHotReloaded())
		{
			createIconTexture();
		}
#endif
	}

	void MarkEnemy(const RectF& rect)
	{
		m_markedEnemies.push_back(rect);
	}

private:
	void createIconTexture()
	{
		const auto image = Icon::CreateImage(0xF11CF, iconSize.x);
		m_iconTexture = Texture{image};
	}
};

namespace Play
{
	UiDangerMarker::UiDangerMarker() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiDangerMarker::Init()
	{
		p_impl->Init();
	}

	void UiDangerMarker::Update()
	{
		ActorBase::Update();
		p_impl->Update(*this);
	}

	void UiDangerMarker::MarkEnemy(const RectF& rect)
	{
		p_impl->MarkEnemy(rect);
	}
}
