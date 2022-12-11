#pragma once
#include "../GUIWidget.h"

class GUIDropDown : public GUIWidget
{
public:

	virtual glm::ivec2 prepare(glm::ivec2 wpos, glm::ivec2 wsize, glm::ivec4 viewport, GUIInput* gui_input) override;
	virtual void draw(NVGcontext* ctx, GUISkin* skin) override;
};
