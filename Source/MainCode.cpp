///////////////////////////////////////////////////////////////////////////////
// maincode.cpp
// ============
// gets called when application is launched - initializes GLEW, GLFW
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
	// Macro for window title
	const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones";

	// Main GLFW window
	GLFWwindow* g_Window = nullptr;

	// scene manager object for managing the 3D scene prepare and render
	SceneManager* g_SceneManager = nullptr;
	// shader manager object for dynamic interaction with the shader code
	ShaderManager* g_ShaderManager = nullptr;
	// view manager object for managing the 3D view setup and projection to 2D
	ViewManager* g_ViewManager = nullptr;
}

// Function declarations
bool InitializeGLFW();
bool InitializeGLEW();

/***********************************************************
 *  main(int, char*)
 ***********************************************************/
int main(int argc, char* argv[])
{
	// Initialize GLFW
	if (InitializeGLFW() == false)
	{
		return(EXIT_FAILURE);
	}

	// Create Shader and View Managers
	g_ShaderManager = new ShaderManager();
	g_ViewManager = new ViewManager(g_ShaderManager);

	// Create the Window (This makes the OpenGL context current)
	g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

	// Initialize GLEW (Must happen AFTER window creation but BEFORE texture loading)
	if (InitializeGLEW() == false)
	{
		return(EXIT_FAILURE);
	}

	// LOAD TEXTURES (Safe to call now that GLEW is initialized)
	g_ViewManager->LoadSceneTextures();

	// Load Shaders
	g_ShaderManager->LoadShaders(
		"../../Utilities/shaders/vertexShader.glsl",
		"../../Utilities/shaders/fragmentShader.glsl");
	g_ShaderManager->use();

	// Initialize SceneManager 
	// IMPORTANT: Passing both ShaderManager AND ViewManager so textures are accessible
	g_SceneManager = new SceneManager(g_ShaderManager, g_ViewManager);
	g_SceneManager->PrepareScene();

	// Main Render Loop
	while (!glfwWindowShouldClose(g_Window))
	{
		glEnable(GL_DEPTH_TEST);

		// Clear buffers (Dark background helps see the 3D scene clearly)
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Prepare Camera/View
		g_ViewManager->PrepareSceneView();

		// Render the 3D Objects
		g_SceneManager->RenderScene();

		glfwSwapBuffers(g_Window);
		glfwPollEvents();
	}

	// Cleanup
	if (NULL != g_SceneManager) { delete g_SceneManager; g_SceneManager = NULL; }
	if (NULL != g_ViewManager) { delete g_ViewManager;  g_ViewManager = NULL; }
	if (NULL != g_ShaderManager) { delete g_ShaderManager; g_ShaderManager = NULL; }

	exit(EXIT_SUCCESS);
}

/***********************************************************
 *	InitializeGLFW()
 ***********************************************************/

bool InitializeGLFW()
{
	if (!glfwInit()) return false;

#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	return true;
}

/***********************************************************
 *	InitializeGLEW()
 ***********************************************************/

bool InitializeGLEW()
{

	glewExperimental = GL_TRUE;
	GLenum GLEWInitResult = glewInit();
	if (GLEW_OK != GLEWInitResult)
	{
		std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
		return false;
	}

	return true;

	std::cout << "INFO: OpenGL Successfully Initialized\n";
	std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;
	return true;
}
