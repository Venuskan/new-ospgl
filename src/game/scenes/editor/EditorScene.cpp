#include "EditorScene.h"
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>
#include <OSP.h>
#include <renderer/Renderer.h>
#include <renderer/lighting/EnvMap.h>
#include <game/GameState.h>

void EditorScene::load()
{

	debug_drawer->debug_enabled = true;

	Renderer* r = osp->renderer;

	r->cam = &cam;


	r->add_drawable(&vehicle);

	r->add_drawable(&sky);
	sky.cubemap.get_noconst()->generate_ibl_irradiance(32, 32);
	r->set_ibl_source(sky.cubemap.data);

	sun = SunLight(0, r->quality.sun_shadow_size);
	sun.color = glm::vec3(1.0);
	sun.position = glm::dvec3(1000.0, 0.0, 0.0);
	sun.ambient_color = glm::vec3(0.4, 0.4, 0.4);
	sun.near_shadow_span = 10.0;

	r->add_light(&sun);

	EnvMap* env = new EnvMap();
	r->add_light(env);

	// Create the bullet physics stuff
	bt_brf_interface = new btDbvtBroadphase();
	bt_collision_config = new btDefaultCollisionConfiguration();
	bt_dispatcher = new btCollisionDispatcher(bt_collision_config);
	bt_world = new btCollisionWorld(bt_dispatcher, bt_brf_interface, bt_collision_config);
	debug_draw = new BulletDebugDrawer();
	debug_draw->setDebugMode(
			btIDebugDraw::DBG_DrawConstraints |
			btIDebugDraw::DBG_DrawWireframe |
			btIDebugDraw::DBG_DrawFrames |
			btIDebugDraw::DBG_DrawConstraintLimits |
			btIDebugDraw::DBG_DrawAabb);
	bt_world->setDebugDrawer(debug_draw);

	lua_core->load(lua_state, "__UNDEFINED__");
	vehicle.init(&lua_state);

	gui.vg = osp->renderer->vg;
	gui.init(this);

	osp->game_state->universe.paused = true;
}

double t = 0.0;

void EditorScene::update()
{
	glm::ivec4 screen = glm::ivec4(0, 0, osp->renderer->get_width(true), osp->renderer->get_height(true));

	gui_input.update();
	gui_screen.new_frame(screen);

	if(cam.blocked)
	{
		gui_input.ext_mouse_blocked = true;
		gui_input.ext_keyboard_blocked = true;
	}

	vehicle.update(osp->game_dt);
	vehicle_int.update(osp->game_dt);

	glm::dvec4 viewport = get_viewport();
	glm::dvec2 real_screen_size =
			glm::dvec2(osp->renderer->get_width(true), osp->renderer->get_height(true));
	int rw = (viewport.z - viewport.x) * (int)real_screen_size.x;
	int rh = (viewport.w - viewport.y) * (int)real_screen_size.y;

	do_gui();
	gui_screen.prepare_pass();

	float gw = (float)gui.get_panel_width();
	glm::vec4 gui_vport = glm::vec4(gw, 0, real_screen_size.x - gw, real_screen_size.y);
	vehicle_int.do_interface(cam.get_camera_uniforms(rw, rh),
							 viewport, gui_vport, real_screen_size, osp->renderer->vg, &gui_input, gui_screen.skin);
	cam.update(osp->game_dt, &gui_input);
	gui_input.ext_mouse_blocked |= cam.blocked;
	gui_input.ext_keyboard_blocked |= cam.blocked;

	gui_screen.input_pass();
	gui_screen.draw();

	bt_world->updateAabbs();

	//bt_world->debugDrawWorld();
}

void EditorScene::render()
{
	// The GUI takes a big portion of the screen and it's opaque
	osp->renderer->override_viewport = get_viewport();
	t += osp->game_dt;

	osp->renderer->render(nullptr);

}

void EditorScene::do_gui()
{
	int width = osp->renderer->get_width(true);
	int height = osp->renderer->get_height(true);
	gui.do_backgrounds(width, height);
	gui.add_canvas(width, height);
}

void EditorScene::do_edveh_gui()
{
}

void EditorScene::unload()
{
	osp->renderer->remove_drawable(&vehicle);
	osp->renderer->override_viewport = glm::dvec4(0.0, 0.0, 1.0, 1.0);
	osp->game_state->universe.paused = false;

	// Destroy the bullet stuff
	delete bt_world;
	delete bt_brf_interface;
	delete bt_dispatcher;
	delete bt_collision_config;
}

EditorScene::EditorScene() : vehicle(this), vehicle_int(&vehicle, &cam),
	sky(std::move(AssetHandle<Cubemap>("debug_system:skybox.hdr")))
{
	gui_screen.init(glm::ivec4(0, 0, osp->renderer->get_width(true), osp->renderer->get_height(true)),
					&skin, &gui_input);
}

glm::dvec4 EditorScene::get_viewport()
{
	double w = (double)gui.get_panel_width() / (double)osp->renderer->get_width(true);
	glm::dvec4 useful = glm::dvec4(w, 0.0, 1.0, 1.0);
	// The interface may apply a multiplier to this "useful" space
	glm::dvec4 sub = vehicle_int.current_interface->get_vehicle_viewport();
	glm::dvec4 result;
	result.x = useful.x + sub.x*(1.0 - useful.x);
	result.y = useful.y + sub.y*(1.0 - useful.y);
	result.z = useful.x + sub.z * (useful.z - useful.x);
	result.w = useful.y + sub.w * (useful.w - useful.y);

	return result;
}


