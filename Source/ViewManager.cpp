///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//  Updated: Christian Tran
//	Date: 11/24/2024
//	Description: 
//	Applied camera navigation with mouse and keybinds for 3D scene
//	Created a perspective and orthographic display of 3D scene
//////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// Variables for mouse and keyboard input
	float gCameraSpeedMultiplier = 1.0f; // Default speed multiplier

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
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
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
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// Register the mouse scroll callback
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}


/**************************************************************
 * Mouse_Position_Callback()
 * Handles mouse movement to change the orientation of the camera.
 * Calculates offset from the previous frame's mouse position
 * and adjusts camera pitch and yaw accordingly.
 **************************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// Initialize the first mouse position
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// Calculate mouse offset since the last frame
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; // Reversed since y-coordinates increase from bottom to top
	gLastX = xMousePos;
	gLastY = yMousePos;

	// Apply sensitivity adjustment
	float sensitivity = 2.5f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Pass offsets to camera to update its direction
	g_pCamera->ProcessMouseMovement(xOffset, yOffset);

	// Adjust movement speed multiplier based on scroll input
	static float gCameraSpeedMultiplier = 1.0f; // Default speed multiplier

	gCameraSpeedMultiplier += static_cast<float>(yOffset) * 0.1f; // Increment/decrement by 10% per scroll
	if (gCameraSpeedMultiplier < 0.1f) // Prevent negative or too-slow speeds
		gCameraSpeedMultiplier = 0.1f;
	if (gCameraSpeedMultiplier > 10.0f) // Cap speed multiplier to a maximum
		gCameraSpeedMultiplier = 10.0f;


	// Log or use this value for debugging
	std::cout << "Camera Speed Multiplier: " << gCameraSpeedMultiplier << std::endl;
}

/*************************************************************
 * Mouse_Scroll_Callback()
 *  Handles mouse scroll events to adjust the speed of camera movement.
 *************************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	// Adjust movement speed multiplier based on scroll input
	static float gCameraSpeedMultiplier = 1.0f; // Default speed multiplier

	gCameraSpeedMultiplier += static_cast<float>(yOffset) * 0.1f; // Increment/decrement by 10% per scroll
	if (gCameraSpeedMultiplier < 0.1f) // Prevent negative or too-slow speeds
		gCameraSpeedMultiplier = 0.1f;
	if (gCameraSpeedMultiplier > 10.0f) // Cap speed multiplier to a maximum
		gCameraSpeedMultiplier = 10.0f;

	// Log or use this value for debugging
	std::cout << "Camera Speed Multiplier: " << gCameraSpeedMultiplier << std::endl;
}



/******************************************************************
 * ProcessKeyboardEvents()
 * Handles keyboard input for camera movement. Allows navigation
 * using WASD for horizontal movement and QE for vertical movement.
 ******************************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// Calculate the camera speed based on frame time
	float cameraSpeed = gDeltaTime * 5.0f * gCameraSpeedMultiplier; // Adjust this multiplier to change speed

	// Move camera forward with W key
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		g_pCamera->Position += cameraSpeed * g_pCamera->Front;

	// Move camera backward with S key
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		g_pCamera->Position -= cameraSpeed * g_pCamera->Front;

	// Move camera left with A key
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		g_pCamera->Position -= glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * cameraSpeed;

	// Move camera right with D key
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		g_pCamera->Position += glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * cameraSpeed;

	// Move camera upward with Q key
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		g_pCamera->Position += cameraSpeed * g_pCamera->Up;

	// Move camera downward with E key
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		g_pCamera->Position -= cameraSpeed * g_pCamera->Up;

	// Close the window if the Escape key is pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_pWindow, true);
}



/***********************************************************
 *  PrepareSceneView()
 *
 * Prepares the scene by calculating the view and projection matrices.
 * Allows toggling between perspective and orthographic projection
 * using the O and P keys.
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix
	projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

	// Check for key presses to toggle projection mode
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
		bOrthographicProjection = true;
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
		bOrthographicProjection = false;

	// Set the projection matrix based on the current mode
	if (bOrthographicProjection)
	{
		// Orthographic projection: directly looking at the object
		projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	}
	else
	{
		// Perspective projection: realistic 3D view
		projection = glm::perspective(glm::radians(g_pCamera->Zoom),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
			0.1f, 100.0f);
	}

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}

