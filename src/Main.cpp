#include <sol.hpp>
#include <iostream>
#include "util/Logger.h"
#include "util/Timer.h"
#include "util/DebugDrawer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "util/InputUtil.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "tools/planet_editor/PlanetEditor.h"
#include "universe/PlanetarySystem.h"
#include "universe/Date.h"
InputUtil* input;

/*
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	ImGuiIO& io = ImGui::GetIO();
	//io.InputQueueCharacters.push_back(codepoint);
	io.AddInputCharacter(codepoint);
}
*/

int main(void)
{
	int width = 1366;
	int height = 768;

	create_global_logger();

	logger->info("Starting OSP");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(width, height, "New OSP", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logger->fatal("Could not initialize glad");
	}

	create_global_asset_manager();
	create_global_debug_drawer();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark();

	// Font for the code editor
	ImFont* font_default = io.Fonts->AddFontDefault();
	ImFont* font_code = io.Fonts->AddFontFromFileTTF("./res/FiraCode-Regular.ttf", 16.0f);

	Timer dtt = Timer();
	double dt = 0.0;
	double t = 0.0;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	PlanetEditor editor = PlanetEditor(window, "earth");

	input = new InputUtil();
	input->setup(window);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	PlanetarySystem system;
	SerializeUtil::read_file_to("res/systems/test_system.toml", system);

	system.compute_sois(0.0);
	system.draw_debug = true;

	Date start_date = Date(2000, Date::DECEMBER, 21);

	t = start_date.to_seconds();

	logger->info("Starting at: {}", start_date.to_string());

	while (!glfwWindowShouldClose(window))
	{
		input->update(window);

		glfwGetWindowSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glfwPollEvents();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		editor.update((float)dt, font_code);



		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		
		//system.render(t, width, height);
		editor.render(width, height);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
		glfwSwapBuffers(window);


		dt = dtt.restart();
		t += dt * 1.0;
	}

	logger->info("Ending OSP");

	delete input;

	glfwDestroyWindow(window);
	glfwTerminate();

	destroy_global_asset_manager();
	destroy_global_logger();

}