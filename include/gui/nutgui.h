#pragma once

#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "nx/sddirectory.h"
#include "nx/install.h"
#include "nx/pfs0.h"
#include "gui/window.h"
#include "nx/buffer.h"

struct Theme
{
	string BackgroundPath;
	string FontPath;
	RGBA TextColor;
	RGBA SelectedTextColor;
};

class Menu : public Window
{
public:
	Menu(Window* p, string id, Rect r) : Window(p, id, r)
	{
		m_selectedIndex = 0;
	}

	u64 keysDown(u64 keys) override
	{
		if (keys & KEY_UP)
		{
			if (m_selectedIndex)
			{
				m_selectedIndex--;
			}
			invalidate();
		}

		if (keys & KEY_DOWN)
		{
			if (m_selectedIndex < m_tabs.size() - 1)
			{
				m_selectedIndex++;
			}
			invalidate();
		}
		return keys;
	}

	sptr<Window>& activePanel()
	{
		return m_panels[m_selectedIndex];
	}

	void draw() override
	{
		int y = 20;
		for (u32 i = 0; i < m_tabs.size(); i++)
		{
			if (i == m_selectedIndex)
			{
				if (isFocused())
				{
					drawRect(0, y-16, width(), 50, txtcolor);
				}
				drawText(20, y, selcolor, m_tabs[i], fntMedium);
				activePanel()->draw();
			}
			else
			{
				drawText(20, y, txtcolor, m_tabs[i], fntMedium);
			}
			y += 50;
		}
	}

	void add(string tab, Window* panel)
	{
		m_tabs.push(tab);
		m_panels.push(sptr<Window>(panel));
	}

protected:
	Buffer<string> m_tabs;
	Buffer<sptr<Window>> m_panels;
	u32 m_selectedIndex;
};

class Body : public Window
{
public:
	Body(Window* p, string id, Rect r) : Window(p, id, r)
	{
	}
};

template<class T>
class HListWnd : public Window
{
public:
	HListWnd(Window* p, string id, Rect r) : Window(p, id, r)
	{
		m_selectedIndex = 0;
		m_offset = 0;
	}

	virtual void drawItem(int itemIndex, int displayIndex, T& item)
	{
		int y = 20 + (displayIndex * rowHeight());

		if (itemIndex == m_selectedIndex)
		{
			if (isFocused())
			{
				drawRect(0, y - 16, width(), 50, txtcolor);
			}
			drawText(20, y, selcolor, (string)item, fntMedium);
		}
		else
		{
			drawText(20, y, txtcolor, (string)item, fntMedium);
		}
	}

	void draw() override
	{
		const int maxLines = 11;

		if (m_selectedIndex >= m_offset + maxLines)
		{
			m_offset = m_selectedIndex - maxLines + 1;
		}

		if (m_selectedIndex < m_offset)
		{
			m_offset = m_selectedIndex;
		}

		for (int i = 0; i < maxLines && i + m_offset < items().size(); i++)
		{
			drawItem(i + m_offset, i, items()[i + m_offset]);
		}
	}

	virtual void select(u32 selected)
	{
		(void)selected;
	}

	u64 keysDown(u64 keys) override
	{
		if (keys & KEY_UP)
		{
			if (m_selectedIndex)
			{
				m_selectedIndex--;
			}
			invalidate();
		}

		if (keys & KEY_DOWN)
		{
			if (items().size() && m_selectedIndex < items().size() - 1)
			{
				m_selectedIndex++;
			}
			invalidate();
		}

		if (keys & KEY_A && items().size())
		{
			select(m_selectedIndex);
		}
		return keys;
	}

	void onFocus() override
	{
		Window::onFocus();
		refresh();
		invalidate();
	}

	virtual void refresh()
	{
	}

	Array<T>& items() { return m_items; }
	u32& rowHeight() { return m_rowHeight; }

	Array<T> m_items;
	u32 m_selectedIndex;
	u32 m_offset;
	u32 m_rowHeight = 50;
};

class SdWnd : public HListWnd<string>
{
public:
	SdWnd(Window* p, string id, Rect r) : HListWnd(p, id, r), dir("/")
	{
	}

	void select(u32 i) override
	{
		(void)i;

		Pfs0 nsp;
		string name = string("/") + items()[m_selectedIndex];
		if (nsp.open(name))
		{
			nsp.install();
		}
	}

	void refresh() override
	{
		string nspExt(".nsp");
		items().resize(0);
		for (auto& f : dir.files())
		{
			if (f->name().endsWith(nspExt))
			{
				items().push(f->name());
			}
		}
		//m_files = dir.files().find(".nsp");
	}

	SdDirectory dir;
};

class TitleRow
{
public:
	TitleRow()
	{
	}

	TitleRow(TitleId titleId, string name) : m_titleId(titleId), m_name(name)
	{
	}

	operator string()
	{
		return hx(titleId()) + "    " + name();
	}

	TitleId& titleId() { return m_titleId; }
	string& name() { return m_name; }

private:
	TitleId m_titleId;
	string m_name;
};

class TicketWnd : public HListWnd<TitleRow>
{
public:
	TicketWnd(Window* p, string id, Rect r) : HListWnd(p, id, r)
	{
	}

	/*void drawItem(int itemIndex, int displayIndex, TitleRow& item) override
	{
		auto& row = items()[itemIndex];
		int y = 20 + (displayIndex * rowHeight());

		if (itemIndex == m_selectedIndex)
		{
			if (isFocused())
			{
				drawRect(0, y - 16, width(), 50, txtcolor);
			}
			drawText(20, y, selcolor, hx(row.titleId()), fntMedium);
			drawText(20 + 100, y, selcolor, row.name(), fntMedium);
		}
		else
		{
			drawText(20, y, txtcolor, hx(row.titleId()), fntMedium);
			drawText(20 + 100, y, txtcolor, row.name(), fntMedium);
		}
	}*/

	void refresh() override
	{
		Buffer<RightsId> rightsIds;
		u32 rightsIdCount = 0;
		u32 installedTicketCount = 0;
		u8 titleKey[16];

		items().resize(0);


		if (esCountCommonTicket(&installedTicketCount))
		{
			error("Failed to count common tickets\n");
			return;
		}

		rightsIds.resize(installedTicketCount);
		memset(rightsIds.buffer(), NULL, rightsIds.sizeBytes());

		if (esListCommonTicket(&rightsIdCount, rightsIds.buffer(), rightsIds.sizeBytes()))
		{
			error("Failed to list common tickets\n");
			return;
		}

		for (unsigned int i = 0; i < rightsIdCount; i++)
		{
			items().push(TitleRow(rightsIds[i].titleId(), getBaseTitleName(rightsIds[i].titleId())));
		}
	}
};

class NutWnd : public Window
{
public:
	NutWnd(Window* p, string id, Rect r) : Window(p, id, r)
	{
	}
};

class ConsoleWnd : public Window
{
public:
	ConsoleWnd(Window* p, string id, Rect r) : Window(p, id, r)
	{
	}
};

class NutGui : public Window
{
public:
	NutGui(string Title, string Footer, Theme Theme) : m_focus(0), Window(string("root"), Rect(0, 0, 1280, 720))
	{
		Rect panelRect(411, 88, 1249 - 411, 646 - 88);

		menu = (Menu*)windows().push(sptr<Window>(new Menu(this, string("menu"), Rect(30, 88, 410 - 30, 646 - 88)))).get();
		body = (Body*)windows().push(sptr<Window>(new Body(this, string("body"), Rect(411, 88, 1249 - 411, 646 - 88 )))).get();

		menu->add(string("SD"), new SdWnd(this, string("SD"), panelRect));
		menu->add(string("CDN"), new NutWnd(this, string("CDN"), panelRect));
		menu->add(string("NUT"), new NutWnd(this, string("NUT"), panelRect));
		menu->add(string("Tickets"), new TicketWnd(this, string("Tickets"), panelRect));
		menu->add(string("Console"), new ConsoleWnd(this, string("CONSOLE"), panelRect));

		setFocus(menu);


		romfsInit();
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_CreateWindowAndRenderer(1280, 720, 0, &_window, &_renderer);
		_surface = SDL_GetWindowSurface(_window);
		SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
		IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP | IMG_INIT_TIF);
		TTF_Init();

		SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

		ttf = Theme.FontPath;
		fntLarge = TTF_OpenFont(Theme.FontPath.c_str(), 25);
		fntMedium = TTF_OpenFont(Theme.FontPath.c_str(), 20);
		bgs = surfInit(Theme.BackgroundPath);
		bgt = texInit(bgs);
		txtcolor = { Theme.TextColor.R, Theme.TextColor.G, Theme.TextColor.B, Theme.TextColor.A };
		selcolor = { Theme.SelectedTextColor.R, Theme.SelectedTextColor.G, Theme.SelectedTextColor.B, Theme.SelectedTextColor.A };
		title = Title;
		footer = Footer;
	}

	~NutGui()
	{
		TTF_Quit();
		IMG_Quit();
		SDL_DestroyRenderer(_renderer);
		SDL_FreeSurface(_surface);
		SDL_DestroyWindow(_window);
		SDL_Quit();
		romfsExit();
	}

	void setFocus(Window* w)
	{
		if (focus())
		{
			if (focus() == w)
			{
				return;
			}
			focus()->onDefocus();
		}
		focus() = w;
		focus()->onFocus();
	}

	u64 keysDown(u64 keys) override
	{
		if (focus())
		{
			keys = focus()->keysDown(keys);
		}

		if (keys & KEY_LEFT)
		{
			if (focus() != menu)
			{
				setFocus(menu);
				invalidate();
			}
		}

		if (keys & KEY_RIGHT)
		{
			if (focus() == menu)
			{
				setFocus(menu->activePanel().get());
				invalidate();
			}
		}

		if (keys & KEY_A && focus() == menu)
		{
			setFocus(menu->activePanel().get());
			invalidate();
		}


		return keys;
	}

	u64 keysUp(u64 keys) override
	{
		if (focus())
		{
			keys = focus()->keysUp(keys);
		}

		return keys;
	}

	bool loop()
	{
		flushGraphics();

		u64 kDown = keysDown(PressedInput);

		if (kDown & KEY_PLUS)
		{
			return false;
		}

		if (kDown & KEY_B)
		{
			if (focus() == menu)
			{
				return false;
			}

			setFocus(menu);
			invalidate();
		}

		if (m_redraw)
		{
			renderGraphics();
			m_redraw = false;
		}

		return true;
	}

	/*
	void render()
	{
		hidScanInput();

		HeldInput = hidKeysHeld(CONTROLLER_P1_AUTO);
		PressedInput = hidKeysDown(CONTROLLER_P1_AUTO);
		ReleasedInput = hidKeysUp(CONTROLLER_P1_AUTO);

		keysDown(PressedInput);

		SDL_RenderClear(_renderer);
		drawBack(bgs, bgt);
		drawText(titleX, titleY, txtcolor, title, fntLarge);



		
		drawText(footerX, footerY, txtcolor, footer, fntLarge);
		SDL_RenderPresent(_renderer);
	}*/

	void renderGraphics()
	{
		SDL_RenderClear(_renderer);
		drawBack(bgs, bgt);
		drawText(titleX, titleY, txtcolor, title, fntLarge);

		if (menu)
		{
			menu->draw();
		}


		drawText(footerX, footerY, txtcolor, footer, fntLarge);
		SDL_RenderPresent(_renderer);
	}

	void flushGraphics()
	{
		if (Frame == 0)
		{
			renderGraphics();
		}

		Frame++;
		hidScanInput();
		HeldInput = hidKeysHeld(CONTROLLER_P1_AUTO);
		PressedInput = hidKeysDown(CONTROLLER_P1_AUTO);
		ReleasedInput = hidKeysUp(CONTROLLER_P1_AUTO);
	}

	Window*& focus() { return m_focus; }
	Buffer<sptr<Window>>& windows() { return m_windows; }

	SDL_Window *_window;

	SDL_Surface *bgs;
	SDL_Texture *bgt;
	string title;
	string footer;

	int titleX = 60;
	int titleY = 30;
	int footerX = titleX;
	int footerY = 672;
	int optX = 55;
	int optY = 115;

	u64 HeldInput = 0;
	u64 PressedInput = 0;
	u64 ReleasedInput = 0;

	// Current frame/iteration, incremented by "flushGraphics" every loop
	int Frame = 0;

	static Theme HorizonLight()
	{
		return{ "romfs:/Graphics/Background.Light.png", "romfs:/Fonts/NintendoStandard.ttf",{ 0, 0, 0, 255 },{ 140, 140, 140, 255 } };
	}

	static Theme HorizonDark()
	{
		return{ "romfs:/Graphics/Background.Dark.png", "romfs:/Fonts/NintendoStandard.ttf",{ 255, 255, 255, 255 },{ 140, 140, 140, 255 } };
	}

	// Theme being used by the current console
	static Theme HorizonCurrent()
	{
		ColorSetId id;
		setsysInitialize();
		setsysGetColorSetId(&id);
		setsysExit();
		if (id == ColorSetId_Dark) return HorizonDark();
		else return HorizonLight();
	}

protected:
	Buffer<sptr<Window>> m_windows;
	Window* m_focus;
	Menu* menu;
	Body* body;
};