#include "stdafx.h"
#include "TitleScene.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "TitleBackground.h"
#include "TitleHud.h"
#include "Util/ActorContainer.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"


namespace
{
	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"title.scene." + key);
	}

	struct RasterScrollCb
	{
		float phase;
		float amplitude;
		float freq;
	};
}

struct Title::TitleScene::Impl
{
	bool m_concluded{};
	TitleBackground m_bg{};
	TitleHud m_hud{};
	RenderTexture m_renderTexture{};
	ConstantBuffer<RasterScrollCb> m_rasterScrollCb{};
	double m_transitionAlpha{};

	void Init(ActorBase& self, const Sukuu::GameSavedata& savedata)
	{
		m_bg = self.AsParent().Birth(TitleBackground());
		m_bg.Init();

		m_hud = self.AsParent().Birth(TitleHud());
		m_hud.Init(savedata);

		m_renderTexture = RenderTexture(Scene::Size());

		StartCoro(self, [this, self](YieldExtended yield)
		{
			startProcess(yield, self);
		});
	}

	void Update(ActorBase& self)
	{
		if (m_transitionAlpha > 0)
		{
			updateWhileTransition(self);
		}
		else
		{
			self.ActorBase::Update();
		}

		// if (MouseL.down()) m_concluded = true;
	}

private:
	void updateWhileTransition(ActorBase& self)
	{
		[&]
		{
			const ScopedRenderTarget2D rs{m_renderTexture.clear(Palette::Black)};
			self.ActorBase::Update();
		}();

		[&]
		{
			m_rasterScrollCb->phase += GetDeltaTime();

			const ScopedCustomShader2D shader{PixelShaderAsset(AssetKeys::PsRasterScroll)};
			Graphics2D::SetPSConstantBuffer(1, m_rasterScrollCb);

			(void)m_renderTexture.draw();
		}();

		(void)Rect(Scene::Size()).draw(ColorF{Constants::HardDarkblue, m_transitionAlpha});
	}

	void startProcess(YieldExtended& yield, ActorView self)
	{
		openTransition(yield, self);

		yield.WaitForTime(0.3);

		m_hud.SetShowPrompt(true);
		yield.WaitForTrue([this]()
		{
			return
				not m_hud.IsCreditHovered() &&
				IsSceneLeftClicked();
		});
		m_hud.SetShowPrompt(false);
		AudioAsset(AssetSes::title_start).playOneShot();

		closeTransition(yield, self);

		yield.WaitForTime(0.3);

		m_concluded = true;
	}

	void openTransition(YieldExtended& yield, ActorView self)
	{
		constexpr double alphaDuration = 2.0;
		m_rasterScrollCb->freq = 10;
		m_rasterScrollCb->amplitude = 1.0;
		m_transitionAlpha = 1.0;
		m_bg.SetCameraTimescale(0.5);

		AnimateEasing<EaseOutCirc>(self, &m_rasterScrollCb->amplitude, 0.0f, alphaDuration);
		AnimateEasing<EaseOutCirc>(self, &m_rasterScrollCb->freq, 2.0f, alphaDuration);
		yield.WaitForDead(AnimateEasing<EaseOutSine>(self, &m_transitionAlpha, 0.0, alphaDuration));

		m_bg.SetCameraTimescale(1.0);
	}

	void closeTransition(YieldExtended& yield, ActorView self)
	{
		constexpr double alphaDuration = 1.5;
		AnimateEasing<EaseOutCirc>(self, &m_rasterScrollCb->amplitude, 1.0f, alphaDuration);
		AnimateEasing<EaseOutCirc>(self, &m_rasterScrollCb->freq, 10.0f, alphaDuration);
		yield.WaitForDead(AnimateEasing<EaseOutSine>(self, &m_transitionAlpha, 1.0, alphaDuration));
	}
};

namespace Title
{
	TitleScene::TitleScene() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TitleScene::Init(const Sukuu::GameSavedata& savedata)
	{
		p_impl->Init(*this, savedata);
	}

	void TitleScene::Update()
	{
		p_impl->Update(*this);
	}

	void TitleScene::PerformReincarnate()
	{
		p_impl->m_bg.ReincarnatePlayer();
	}

	bool TitleScene::IsConcluded() const
	{
		return p_impl->m_concluded;
	}
}
