﻿#pragma once
#include "ui/menu_helper.hpp"
#include "graphics/texture_atlas.hpp"

class TitleScene;

class TitleMenu : public IMenuEventHandler
{
private:
	TitleScene* m_pTitleScene;

	Menu m_menu;

	TextureAtlas m_menuItemTextureAtlas;

	Texture m_menuCursorTexture;

	Stopwatch m_stopwatch;

	double m_easedCursorPos = 0.0;

public:
	enum Item : int32
	{
		kStart = 0,
		kOption,
		kInputGate,
		kExit,

		kItemMax,
	};

	explicit TitleMenu(TitleScene* pTitleScene);

	void update();

	void draw() const;

	virtual void processMenuEvent(const MenuEvent& event) override;
};
