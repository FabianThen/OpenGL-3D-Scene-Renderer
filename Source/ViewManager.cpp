///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>  
#include "stb_image.h"
#include <iostream>

// camera object used for viewing and interacting with
// the 3D scene
Camera* g_pCamera = nullptr;

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/


void ViewManager::LoadSceneTextures()
{
	// Make sure these variable names match exactly what is in ViewManager.h private section
	m_textureID = LoadTextureFromFile("textures/pine.jpg");
	m_detailTextureID = LoadTextureFromFile("textures/rug.jpg");
	m_floorTextureID = LoadTextureFromFile("textures/ceramic.jpg");
	m_whiteWoodID = LoadTextureFromFile("textures/wood.jpg");
	m_leatherID = LoadTextureFromFile("textures/leather.jpg");
	m_pRugTextureId = LoadTextureFromFile("textures/rug2.jpg");
	m_fabricID = LoadTextureFromFile("textures/fabric.jpg");
	

	// Check if it worked
	if (m_fabricID == 0) {
		std::cout << "Texture Load Failed!" << std::endl;

	}
}


ViewManager::ViewManager(ShaderManager* pShaderManager)
	: m_pShaderManager(pShaderManager),
	m_pWindow(nullptr),
	m_textureID(0),
	m_detailTextureID(0),
	m_floorTextureID(0),
	m_whiteWoodID(0),
	m_leatherID(0),         
	m_fabricID(0),
	m_pRugTextureId(0),
	m_pRug2TextureId(0)      
{
	g_pCamera = new Camera();

	// Default camera parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80.0f;
	g_pCamera->MovementSpeed = 20.0f;
}


/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/

ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/

GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/

void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// Retrieve the ViewManager instance from the window user pointer
	ViewManager* viewMgr = static_cast<ViewManager*>(glfwGetWindowUserPointer(window));
	if (viewMgr && g_pCamera)
	{
		viewMgr->HandleMouseMovement(xMousePos, yMousePos);
	}
}

/***********************************************************
 *  HandleMouseMovement()
 *
 *  This method processes mouse movement events and updates
 *  the camera orientation accordingly.
 ***********************************************************/

void ViewManager::HandleMouseMovement(double xMousePos, double yMousePos)
{
	if (gFirstMouse)
	{
		gLastX = static_cast<float>(xMousePos);
		gLastY = static_cast<float>(yMousePos);
		gFirstMouse = false;
	}

	float xOffset = static_cast<float>(xMousePos - gLastX);
	float yOffset = static_cast<float>(gLastY - yMousePos); // reversed since y-coordinates go from bottom to top

	gLastX = static_cast<float>(xMousePos);
	gLastY = static_cast<float>(yMousePos);

	if (g_pCamera)
	{
		g_pCamera->ProcessMouseMovement(xOffset, yOffset);
	}
}
/***********************************************************
* Mouse_Scroll_Callback
* **********************************************************/

void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	// Reach into the global namespace variables for the camera
	if (g_pCamera != nullptr)
	{
		// yOffset is the scroll wheel movement
		g_pCamera->ProcessMouseScroll(static_cast<float>(yOffset));
		std::cout << "New Zoom Level: " << g_pCamera->Zoom << std::endl;
	}
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/

void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// process camera zooming in and out
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	}

	// process camera panning left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	}
	// process up/down movement
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gDeltaTime);
	}
	static bool pPressedLastFrame = false;
	static bool oPressedLastFrame = false;

	bool pPressed = glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS;
	bool oPressed = glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS;

	if (pPressed && !pPressedLastFrame)
	{
		bOrthographicProjection = false;
		std::cout << "Switched to Perspective Projection" << std::endl;
	}
	if (oPressed && !oPressedLastFrame)
	{
		bOrthographicProjection = true;
		std::cout << "Switched to Orthographic Projection" << std::endl;
	}

	pPressedLastFrame = pPressed;
	oPressedLastFrame = oPressed;

}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/

void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// Per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// Process any keyboard events that may be waiting in the event queue
	ProcessKeyboardEvents();

	if (bOrthographicProjection)
	{
		// Orthographic projection parameters
		float orthoSize = 10.0f;
		float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

		// Create the orthographic projection matrix
		projection = glm::ortho(
			-orthoSize * aspectRatio, orthoSize * aspectRatio,
			-orthoSize, orthoSize,
			0.1f, 100.0f);

		// Set the orthographic camera: positioned above looking down
		view = glm::lookAt(
			glm::vec3(0.0f, 10.0f, 0.0f),    // Camera position (elevated)
			glm::vec3(0.0f, 0.0f, 0.0f),     // Look at the origin
			glm::vec3(0.0f, 0.0f, -1.0f));   // Up vector pointing towards negative Z
	}
	else
	{
		// Perspective projection matrix
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom),
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f, 100.0f);

		// Get view matrix from the camera
		view = g_pCamera->GetViewMatrix();
	}

	// If the shader manager object is valid
	if (m_pShaderManager != nullptr)
	{
		// Set the view matrix in the shader
		m_pShaderManager->setMat4Value(g_ViewName, view);

		// Set the projection matrix in the shader
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);

		// Set the camera position in the shader (if needed for lighting calculations)
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}

unsigned int ViewManager::LoadTextureFromFile(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);  // Flip vertical for OpenGL texture coords
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		else
			format = GL_RGB;  // fallback

		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Texture wrapping and filtering parameters.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		return 0;
	}

	return textureID;
}