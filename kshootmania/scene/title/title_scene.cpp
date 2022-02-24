﻿#include "title_scene.hpp"
#include "title_assets.hpp"
#include "scene/option/option_scene.hpp"

namespace
{
	constexpr double kExitFadeOutDurationSec = 0.8;
}

TitleScene::TitleScene(const InitData& initData)
	: SceneManager<StringView>::Scene(initData)
	, m_menu(this)
	, m_bgTexture(TextureAsset(TitleTexture::kBG))
	, m_exitStopwatch(StartImmediately::No)
{
}

void TitleScene::update()
{
	if (m_exitStopwatch.isStarted() && m_exitStopwatch.sF() > kExitFadeOutDurationSec)
	{
		System::Exit();
		return;
	}

	m_menu.update();
}

void TitleScene::draw() const
{
	ScreenUtils::FitToHeight(m_bgTexture).drawAt(Scene::Center());
	m_menu.draw();

	// Fadeout before exit
	if (m_exitStopwatch.isStarted())
	{
		const double alpha = m_exitStopwatch.sF() / kExitFadeOutDurationSec;
		Scene::Rect().draw(ColorF{ 0.0, alpha });
	}
}

void TitleScene::processMenuItem(TitleMenu::Item item)
{
	switch (item)
	{
	case TitleMenu::kStart:
		break;

	case TitleMenu::kOption:
		changeScene(OptionScene::kSceneName, SceneManagement::kDefaultTransitionMs);
		break;

	case TitleMenu::kInputGate:
		break;

	case TitleMenu::kExit:
		m_exitStopwatch.start();
		break;
	}
}
