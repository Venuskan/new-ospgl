#include "EditorGUI.h"
#include <glm/glm.hpp>

#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include <gui/widgets/GUITextField.h>

#include "../EditorScene.h"
#include <OSP.h>


int EditorGUI::get_panel_width()
{
	return part_list.get_panel_width();
}

void EditorGUI::do_gui(int width, int height, GUIInput* gui_input)
{
	float w = (float)width; float h = (float)height;

	// Draw the side pane
	float swidth = (float)get_panel_width();

	nvgFillColor(vg, skin.background_color);

	// Panel background
	nvgBeginPath(vg);
	nvgRect(vg, 0.0f, 0.0f, swidth, 1.0f * h);
	nvgFill(vg);

	// Prepare first so gui blocks stuff, but it's drawn last
	window_manager.prepare(gui_input, &skin);
	
	if(edveh_int->attach_interface.selected == nullptr)
	{
		part_list.do_gui(width, get_panel_width(), height, gui_input);
	}
	else
	{
		trashcan.do_gui(width, get_panel_width(), height, gui_input);
	}

	do_toolset(width, height, swidth, gui_input);
	do_file(width, height, gui_input);

	prev_width = width;
	prev_height = height;

	window_manager.viewport = glm::ivec4(0, 0, width, height);
	window_manager.draw(vg, &skin);
}


void EditorGUI::do_toolset(int width, int height, float swidth, GUIInput* gui_input) 
{
	float twidth = 152.0f;
	float theight = 23.0f;
	float real_theight = 30.0f;

	nvgBeginPath(vg);
	nvgMoveTo(vg, swidth, 0.0f);
	nvgLineTo(vg, swidth + twidth + theight, 0.0f);
	nvgLineTo(vg, swidth + twidth, theight);
	nvgLineTo(vg, swidth, theight);
	nvgLineTo(vg, swidth, 0.0f);
	nvgFillColor(vg, skin.background_color);
	nvgFill(vg);

	toolset_canvas.prepare(glm::ivec2(swidth + 4, 0), glm::ivec2(twidth, real_theight), gui_input);
	toolset_canvas.draw(vg, &skin, glm::ivec4(0, 0, width, height));
	
}

void EditorGUI::do_file(int width, int height, GUIInput* gui_input) 
{
	// File canvas background
	float fwidth = 300.0f;
	float fheight = 25.0f;
	nvgBeginPath(vg);
	nvgMoveTo(vg, width, 0.0f);
	nvgLineTo(vg, width - fwidth - fheight, 0.0f);
	nvgLineTo(vg, width - fwidth, fheight);
	nvgLineTo(vg, width, fheight);
	nvgLineTo(vg, width, 0.0f);
	nvgFillColor(vg, skin.background_color);
	nvgFill(vg);

	file_canvas.child_0_pixels = fwidth - 18.0f * 5.0f;

	file_canvas.prepare(glm::ivec2(width - fwidth, 0), glm::ivec2(fwidth, fheight), gui_input);
	file_canvas.draw(vg, &skin, glm::ivec4(0, 0, width, height));
	
}


void EditorGUI::init(EditorScene* sc)
{	
	prev_width = 0;
	prev_height = 0;
	// We hold a pointer to interact with the 3D editor
	this->edveh_int = &sc->vehicle_int;

	part_list.init(sc, vg, &skin);
	trashcan.init(sc, vg, &skin);

	prepare_toolset();
	prepare_file();

}

void EditorGUI::set_editor_mode(EditorMode mode) 
{
	editor_mode = mode;
	
	edveh_int->current_interface->leave();
	switch(mode)
	{
		case ATTACHING:
			edveh_int->current_interface = &edveh_int->attach_interface;
			break;
		case WIRING:	
			edveh_int->current_interface = &edveh_int->wire_interface;
			break;
	}
}

void EditorGUI::prepare_toolset() 
{	
	// Prepare the toolset_canvas
	GUIListLayout* tlayout = new GUIListLayout();
	tlayout->vscrollbar.draw = false;
	toolset_canvas.layout = tlayout;

	auto create_button = [tlayout, this](const std::string& name, EditorMode mode)
	{
		GUIImageButton* button = new GUIImageButton();
		button->set_image(vg, AssetHandle<Image>("core", name));
		button->force_image_size = glm::ivec2(24, 24);
		tlayout->add_widget(button);

		button->on_clicked.add_handler([this, button, mode](int btn)
		{
			if(this->edveh_int->can_change_editor_mode())
			{
				this->current_editor_mode_button->toggled = false;
				this->current_editor_mode_button = button;
				button->toggled = true;
				this->set_editor_mode(mode);
			}
		});

		return button;
	};

	editor_mode = ATTACHING;
	current_editor_mode_button = create_button("editor/attach.png", ATTACHING);
	current_editor_mode_button->toggled = true;
	edveh_int->current_interface = &edveh_int->attach_interface;

	create_button("editor/transform.png", TRANSFORMING);
	create_button("editor/wire.png", WIRING);
	create_button("editor/duct.png", PLUMBING);
	create_button("editor/electric.png", ELECTRIC_WIRING);
}

void EditorGUI::prepare_file() 
{
	// It doesn't matter as we later on set the fixed pixel ammount
	auto pair = file_canvas.divide_h(0.5f);
	GUICanvas* left = pair.first;
	GUICanvas* right = pair.second;

	left->layout = new GUISingleLayout();

	GUIListLayout* list = new GUIListLayout(0, 2);
	list->vscrollbar.draw = false;
	list->margins.x = 1; list->margins.y = 1;
	right->layout = list;

	// Left contains just the text field
	GUITextField* text_field = new GUITextField();
	left->layout->add_widget(text_field);

	// Right contains the small buttons

	auto create_button = [right, this](const std::string& name)
	{
		GUIImageButton* button = new GUIImageButton();
		button->set_image(vg, AssetHandle<Image>("core", name));
		button->force_image_size = glm::ivec2(16, 16);
		right->layout->add_widget(button);
	};

	create_button("editor/new.png");
	create_button("editor/load.png");
	create_button("editor/save.png");
	create_button("editor/launch.png");
	create_button("editor/quit.png");
}
