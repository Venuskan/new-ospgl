#pragma once
#include "../Scene.h"
#include <nanovg/nanovg.h>
#include "EditorCamera.h"
#include "gui/EditorGUI.h"
#include "EditorVehicle.h"
#include "EditorVehicleInterface.h"

#include <assets/BitmapFont.h>

#include <renderer/lighting/SunLight.h>

#include <assets/Model.h>

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#pragma warning(pop)
#include <physics/debug/BulletDebugDrawer.h>
#include <renderer/util/Skybox.h>

class EditorScene : public Scene
{
public:	
	friend class EditorGUI;
	EditorGUI gui;
	Skybox sky;

	glm::dvec4 get_viewport();
	
private:
	SimpleSkin skin;
	EditorCamera cam;


	// We need a world for the very simple colliders, but we have no 
	// dynamics, links, or anything like that
	btDefaultCollisionConfiguration* bt_collision_config;
	btBroadphaseInterface* bt_brf_interface;
	btCollisionDispatcher* bt_dispatcher;
	BulletDebugDrawer* debug_draw;

	SunLight sun;

	void do_gui();
	void do_edveh_gui();

public:
	EditorVehicle vehicle;
	EditorVehicleInterface vehicle_int;

	// We hold a non-universe lua_state for all machines to interact
	sol::state lua_state;
	
	btCollisionWorld* bt_world;

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	virtual void unload() override;
	std::string get_name() override
	{
		return "editor";
	}
	void do_imgui_debug() override
	{

	}

	EditorScene();

};
