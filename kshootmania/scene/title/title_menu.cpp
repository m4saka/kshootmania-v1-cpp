﻿#include "title_menu.hpp"
#include "title_scene.hpp"
#include "title_assets.hpp"

namespace
{
	constexpr int32 kMainTexCol = 0;
	constexpr int32 kSubTexCol = 1;

	constexpr int32 kMenuItemOffsetY = 260;
	constexpr int32 kMenuItemDiffY = 25;

	constexpr double kMenuCursorPosRelaxationTimeSec = 0.03;

	double MenuCursorAlphaValue(double sec)
	{
		constexpr double baseValue = 121.0 / 256;
		constexpr double maxValue = 211.0 / 256;
		constexpr double periodSec = Math::TwoPi * 0.15;
		constexpr double secOffset = 1.0 / 0.15;
		return baseValue + (maxValue - baseValue) * Periodic::Sine0_1(periodSec, sec + secOffset);
	}

	double EaseValue(double currentValue, double targetValue, double relaxationTime)
	{
		const double blendRate = Max(relaxationTime - Scene::DeltaTime(), 0.0) / relaxationTime;
		return Math::Lerp(currentValue, targetValue, blendRate);
	}
}

TitleMenu::TitleMenu(TitleScene* pTitleScene)
	: m_pTitleScene(pTitleScene)
	, m_menu(MenuHelper::MakeVerticalMenu(this, kItemMax))
	, m_menuItemTextureAtlas(TitleTexture::kMenuItem, kItemMax, 2/* <- Additive Texture & Subtractive Texture */)
	, m_menuCursorTexture(TextureAsset(TitleTexture::kMenuCursor))
	, m_stopwatch(StartImmediately::Yes)
{
}

void TitleMenu::update()
{
	m_menu.update();

	m_easedCursorPos = EaseValue(m_easedCursorPos, static_cast<double>(m_menu.cursorIdx()), kMenuCursorPosRelaxationTimeSec);
}

void TitleMenu::draw() const
{
	using namespace ScreenUtils;

	const int32 x = Scene::Center().x;

	// Draw menu cursor (additive)
	{
		const ScopedColorMul2D colorMultiply(MenuCursorAlphaValue(m_stopwatch.sF()));
		const ScopedRenderStates2D additive(BlendState::Additive);
		const TextureRegion textureRegion = Scaled3x(m_menuCursorTexture);
		textureRegion.draw(x - textureRegion.size.x / 2, Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * m_easedCursorPos);
	}

	// Draw menu items
	for (int32 i = 0; i < kItemMax; ++i)
	{
		const int32 y = Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * i;
		{
			// Sub-texture (subtractive)
			const ScopedRenderStates2D subtractive(BlendState::Subtractive);
			const TextureRegion textureRegion = Scaled3x(m_menuItemTextureAtlas(i, kSubTexCol));
			textureRegion.draw(x - textureRegion.size.x / 2, y);
		}
		{
			// Main texture (additive)
			const ScopedRenderStates2D additive(BlendState::Additive);
			const TextureRegion textureRegion = Scaled3x(m_menuItemTextureAtlas(i, kMainTexCol));
			textureRegion.draw(x - textureRegion.size.x / 2, y);
		}
	}
}

void TitleMenu::processMenuEvent(const MenuEvent& event)
{
	switch (event.trigger)
	{
	case MenuEventTrigger::Enter:
		m_pTitleScene->processMenuItem(Item{ event.menuItemIdx });
		break;

	case MenuEventTrigger::Esc:
		m_menu.moveCursorTo(kExit);
		break;

	default:
		break;
	}
}