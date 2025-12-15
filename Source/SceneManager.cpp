///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// loading images from textures folder
	bReturn = CreateGLTexture(
		"textures/whitedesk.jpg", "desk");

	bReturn = CreateGLTexture(
		"textures/monitor.jpg", "monitor");

	bReturn = CreateGLTexture(
		"textures/blackceramic.jpg", "cup");

	bReturn = CreateGLTexture(
		"textures/blackwood.jpg", "pencil");

	bReturn = CreateGLTexture(
		"textures/keyboard.jpg", "keyboard");

	bReturn = CreateGLTexture(
		"textures/whiteceramic.jpg", "pencilcup");

	bReturn = CreateGLTexture(
		"textures/graysmooth.jpg", "mouse");

	bReturn = CreateGLTexture(
		"textures/redcover.jpg", "book1");

	bReturn = CreateGLTexture(
		"textures/bluecover.jpg", "book2");

	bReturn = CreateGLTexture(
		"textures/browncover.jpg", "book3");


	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;


		// find the defined material that matches the tag
		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/*****************************
* Name: Christian Tran
* Date: 12/8/2024
* Course: CS 330
* Description: Milestone six - light scene
*****************************/

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{

	OBJECT_MATERIAL material;

	// Define a white shiny material
	material.tag = "shinyWhite";
	material.ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	material.ambientStrength = 0.2f;
	material.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	material.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	material.shininess = 32.0f;
	m_objectMaterials.push_back(material);

	// Define a blueish reflective material
	material.tag = "blueMaterial";
	material.ambientColor = glm::vec3(0.1f, 0.1f, 0.5f);
	material.ambientStrength = 0.3f;
	material.diffuseColor = glm::vec3(0.2f, 0.2f, 0.8f);
	material.specularColor = glm::vec3(0.5f, 0.5f, 1.0f);
	material.shininess = 64.0f;
	m_objectMaterials.push_back(material);

}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	

	// Enable custom lighting
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// main light source positioned to the left of the cone
	m_pShaderManager->setVec3Value("lightSources[0].position", -5.0f, 5.0f, 5.0f); // Positioned to the left and slightly above
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.05f, 0.05f, 0.05f); // Low ambient light
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.8f, 0.8f, 0.8f); // Bright diffuse light
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 1.0f, 1.0f, 1.0f); // Strong specular highlight
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.5f);

	// Add a secondary light source to ensure the rest of the scene remains darker
	m_pShaderManager->setVec3Value("lightSources[1].position", 0.0f, -3.0f, -5.0f); // Positioned below and to the back
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.01f); // Minimal ambient contribution
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.2f, 0.2f, 0.2f); // Subtle diffuse light
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.0f, 0.0f, 0.0f); // No specular highlight
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 16.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.0f);

	// Added an overhead light for subtle illumination of the top surfaces
	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 8.0f, 0.0f); // Positioned overhead
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.03f, 0.03f, 0.03f); // Very low ambient light
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.3f, 0.3f, 0.3f); // Moderate diffuse light
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.2f, 0.2f, 0.2f); // Weak specular highlight
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 8.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.1f);

}



/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	LoadSceneTextures();

	// define the materials for objects in the scene
	DefineObjectMaterials();
	// add and define the light sources for the scene
	SetupSceneLights();

	// Load meshes for basic shapes (boxes, cylinders, planes, etc.)
	m_basicMeshes->LoadPlaneMesh();    // For the desk surface
	m_basicMeshes->LoadBoxMesh();      // For the keyboard, mouse, and stack of notebooks
	m_basicMeshes->LoadCylinderMesh(); // For the pencil cup and mug
	m_basicMeshes->LoadSphereMesh();   // For any rounded shapes, like parts of the mug

}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 *	Updated: Christian Tran
 *	Date: 11/24/2024
 *	Description: 
 *	Updated positions of every shape for 3D Scene to fit 
 *	with 3D plane (surface of the desk)
 ***********************************************************/
void SceneManager::RenderScene()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// Render the desk (large plane as the surface of the desk)
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f); // Desk dimensions
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f); // Position it slightly below the center
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	// SetShaderColor(0.8f, 0.8f, 0.8f, 1.0f); // Desk surface color (light gray)
	SetShaderTexture("desk"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	SetShaderMaterial("shinyWhite"); // Assign shiny white material

	m_basicMeshes->DrawPlaneMesh(); // Draw desk surface

	// Render the monitor (Screen for the monitor)
	scaleXYZ = glm::vec3(10.0f, 7.0f, 1.0f); // Monitor dimensions (wide screen)
	positionXYZ = glm::vec3(0.0f, 5.0f, -2.0f); // Position it at the center of the desk
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.5f, 0.5f, 0.5f, 1.0f); // Monitor color (gray)
	SetShaderTexture("monitor"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawBoxMesh(); // Draw monitor

	// Render the monitor (Box for the monitor)
	scaleXYZ = glm::vec3(10.5f, 9.0f, 1.0f); // Monitor dimensions (wide screen)
	positionXYZ = glm::vec3(0.0f, 4.5f, -2.2f); // Position it at the center of the desk
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.5f, 0.5f, 0.5f, 1.0f); // Monitor color (gray)

	m_basicMeshes->DrawBoxMesh(); // Draw monitor

	// Render the keyboard (Box for the keyboard)
	scaleXYZ = glm::vec3(8.0f, 0.2f, 2.5f); // Keyboard dimensions
	positionXYZ = glm::vec3(0.0f, 0.1f, 0.0f); // Position it just in front of the monitor
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f); // Keyboard color (dark gray/black)
	SetShaderTexture("keyboard"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawBoxMesh(); // Draw keyboard

	// Render the mouse (Small box for the mouse)
	scaleXYZ = glm::vec3(1.0f, 0.2f, 1.0f); // Mouse dimensions
	positionXYZ = glm::vec3(5.0f, 0.1f, 0.0f); // Position it beside the keyboard
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f); // Mouse color (dark gray/black)
	SetShaderTexture("mouse"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawBoxMesh(); // Draw mouse

	// Render the pencil cup (Cylinder)
	scaleXYZ = glm::vec3(1.0f, 2.0f, 1.0f); // Pencil cup size
	positionXYZ = glm::vec3(8.0f, 0.1f, 0.0f); // Position to the right of the monitor
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f); // Pencil cup color (black)
	SetShaderTexture("pencilcup"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawCylinderMesh(); // Draw pencil cup

	// Render the pencils (thin cylinders)
	scaleXYZ = glm::vec3(0.1f, 2.0f, 0.1f); // Pencil dimensions
	positionXYZ = glm::vec3(7.5f, 1.0f, 0.0f); // Position each pencil in the cup
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f); // Pencil color (black)
	SetShaderTexture("pencil"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawCylinderMesh(); // Draw one pencil
	// Duplicate the cylinder to draw remaining pencils
	for (int i = 1; i < 6; ++i)
	{
		positionXYZ.x += 0.25f; // Offset each pencil a bit
		SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
		m_basicMeshes->DrawCylinderMesh(); // Draw next pencil
	}

	// Render the stack of notebooks (Boxes)
	float notebookWidth = 2.0f;
	scaleXYZ = glm::vec3(notebookWidth, 0.3f, 3.0f); // Notebook dimensions
	positionXYZ = glm::vec3(-8.0f, 0.15f, 0.0f); // Position stack of notebooks to the left of the monitor
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.5f, 1.0f, 1.0f, 1.0f); // Notebook color (Cyan)
	SetShaderTexture("book1"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawBoxMesh(); // Draw first notebook (largest)

	scaleXYZ = glm::vec3(notebookWidth, 0.3f, 2.5f); // Slightly smaller notebook
	positionXYZ.y += 0.3f; // Offset to stack the next notebook on top
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.0f, 0.8f, 0.0f, 1.0f); // Notebook color (Green)
	SetShaderTexture("book2"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawBoxMesh(); // Draw second notebook

	scaleXYZ = glm::vec3(notebookWidth, 0.3f, 2.0f); // Smallest notebook
	positionXYZ.y += 0.3f; // Offset to stack the next notebook on top
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.8f, 0.0f, 0.8f, 1.0f); // Notebook color (pink)
	SetShaderTexture("book3"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawBoxMesh(); // Draw third notebook

	// Render the mug (Cylinder for the body and a small cone for the handle)
	scaleXYZ = glm::vec3(0.5f, 1.5f, 1.5f); // Mug dimensions (wider base)
	positionXYZ = glm::vec3(-5.0f, 0.1f, 0.0f); // Position it close to the monitor
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f); // Mug color (dark gray)
	SetShaderTexture("cup"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawCylinderMesh(); // Draw mug body


	// Draw the mug handle (small cone or cylinder)
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f); // Handle dimensions
	positionXYZ = glm::vec3(-4.5f, 0.6f, 0.0f); // Position it on the side of the mug
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f); // Handle color (gray)
	SetShaderTexture("cup"); // Set texture color
	SetTextureUVScale(1.0, 1.0); // Set texture scale

	m_basicMeshes->DrawCylinderMesh(); // Draw handle

}
