///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

// GLM includes for transformations and math
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <iostream>

extern Camera* g_pCamera;

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";

	//  to handle the tiling in RenderScene
	const char* g_UVScaleName = "uvScale";
}


/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/

SceneManager::SceneManager(ShaderManager* pShaderManager, ViewManager* pViewManager)
{
	m_pShaderManager = pShaderManager;
	m_pViewManager = pViewManager;   // Correctly stores the ViewManager pointer
	m_basicMeshes = new ShapeMeshes();
	m_loadedTextures = 0;            // Initializes the counter to 0
}
/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/

SceneManager::~SceneManager()
{
	// Clear the GPU texture memory
	for (auto& texture : m_textureIDs)
	{
		glDeleteTextures(1, &texture.ID);
	}
	m_textureIDs.clear();

	// Clean up the Mesh object (Logical Flow)
	if (m_basicMeshes != NULL)
	{
		delete m_basicMeshes;
		m_basicMeshes = NULL;
	}

   // Nullify the pointer to the Shader Manager
	m_pShaderManager = NULL;
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

	for (const auto& tex : m_textureIDs) {
		if (tex.tag == tag) return true;
	}

	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);

	if (image)
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Wrapping: Standard GL_REPEAT for tiling floors/walls
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Filtering: Use Mipmaps for the Min Filter to prevent shimmering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else {
			stbi_image_free(image);
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0);

		TEXTURE_INFO newTexture;
		newTexture.ID = textureID;
		newTexture.tag = tag;
		m_textureIDs.push_back(newTexture);

		return true;
	}

	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots. There are up to 16 slots.
 ***********************************************************/

void SceneManager::BindGLTextures()
{
	// Using .size() ensures we always bind the exact number of textures loaded
	for (unsigned int i = 0; i < (unsigned int)m_textureIDs.size(); i++)
	{
		// Activate the corresponding texture unit (0, 1, 2, etc.)
		glActiveTexture(GL_TEXTURE0 + i);

		// Bind the specific texture ID stored in our vector
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
	// Loop through all loaded textures in the vector
	for (unsigned int i = 0; i < (unsigned int)m_textureIDs.size(); i++)
	{
		// Use glDeleteTextures to release GPU memory
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}

	// Clear the vector so the tags and old IDs are gone
	m_textureIDs.clear();
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
	// Use size_t to prevent signed/unsigned mismatch warnings
	std::size_t index = 0;
	bool bFound = false;

	// Loop through the vector to find a matching tag
	while ((index < m_textureIDs.size()) && (bFound == false))
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
	// Use size_t for industry-standard loop safety
	std::size_t index = 0;
	bool bFound = false;

	// Search the vector for the matching tag
	while ((index < m_textureIDs.size()) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			// Return the index (the "slot") where the texture resides
			textureSlot = static_cast<int>(index);
			bFound = true;
		}
		else
			index++;
	}

	return textureSlot;
}


/*************************************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 *************************************************************************/

bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	// Check if the list is empty
	if (m_objectMaterials.size() == 0)
	{
		return false;
	}

	// Use size_t to avoid signed/unsigned mismatch warnings
	std::size_t index = 0;
	bool bFound = false;

	// Search for the matching tag
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag == tag)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	//  Return 'bFound' so the caller knows if the search actually worked
	return bFound;
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
	// Initialize matrices with the Identity Matrix (glm::mat4(1.0f))
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleXYZ);

	glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), positionXYZ);

	// Combine (TRS Order: Translation * Rotation * Scale)
	glm::mat4 modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		// Pass the matrix to the "model" uniform in the shader
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

void SceneManager::SetShaderTexture(std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		int textureSlot = FindTextureSlot(textureTag);

		// Only enable texturing if the tag was actually found in vector
		if (textureSlot != -1)
		{
			m_pShaderManager->setBoolValue(g_UseTextureName, true);
			m_pShaderManager->setSampler2DValue(g_TextureValueName, textureSlot);
		}
		else
		{
			// If not found, default to no texture so the object doesn't turn black
			m_pShaderManager->setBoolValue(g_UseTextureName, false);
			std::cout << "Warning: Texture tag '" << textureTag << "' not found!" << std::endl;
		}
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
		// Use the global constant to ensure consistency across the class
		m_pShaderManager->setVec2Value(g_UVScaleName, glm::vec2(u, v));
	}
}


/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/

void SceneManager::SetShaderMaterial(std::string tag)
{
	OBJECT_MATERIAL material;
	if (FindMaterial(tag, material))
	{
		
		m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor.r, material.diffuseColor.g, material.diffuseColor.b);
		m_pShaderManager->setVec3Value("material.specularColor", material.specularColor.r, material.specularColor.g, material.specularColor.b);
		m_pShaderManager->setFloatValue("material.shininess", material.shininess);
	}
}


/**************************************************************/
/***STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

bool bReturn = false;



/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/

void SceneManager::PrepareScene()
{
	// Load the textures into the m_textureIDs vector
	// (Ensure this method exists and calls CreateGLTexture for your pine, leather, etc.)
	LoadSceneTextures();

	// Load meshes into GPU memory
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();

	// Define the light sources (Directional and Point Lights)
	SetupSceneLights();

	// Fill the m_objectMaterials list for FindMaterial() to work
	DefineObjectMaterials();
}
//************************************************************

void SceneManager::LoadSceneTextures()
{
	// Load each texture and give it a tag that matches the RenderScene calls
	// Ensure the file paths point to the actual texture files
	bool bReturn = false;

	bReturn = CreateGLTexture("../../Utilities/textures/pine.jpg", "pine");
	bReturn = CreateGLTexture("../../Utilities/textures/rug.jpg", "rug");
	bReturn = CreateGLTexture("../../Utilities/textures/leather.jpg", "leather");
	bReturn = CreateGLTexture("../../Utilities/textures/ceramic.jpg", "ceramic");
	bReturn = CreateGLTexture("../../Utilities/textures/wood.jpg", "wood");
    bReturn = CreateGLTexture("../../Utilities/textures/fabric.jpg", "fabric");

	// After loading them into the vector, bind them to the GPU texture units
	BindGLTextures();
}

//*************************************************************
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setBoolValue("bUseLighting", true);

	// --- LIGHT 0: LEFT RECESSED (Ceiling) ---
	m_pShaderManager->setVec3Value("pointLights[0].position", -7.0f, 18.5f, -2.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.05f, 0.05f, 0.03f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);

	// --- LIGHT 1: RIGHT RECESSED (Ceiling) ---
	m_pShaderManager->setVec3Value("pointLights[1].position", 7.0f, 18.5f, -2.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.05f, 0.05f, 0.03f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);

	// --- LIGHT 2: FIREPLACE GLOW (Unique ID!) ---
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[2].position", 0.0f, 3.5f, -6.8f);
	// Lower intensities so it stays localized
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.01f, 0.005f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.3f, 0.15f, 0.0f); // Bright Orange
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.2f, 0.1f, 0.0f);

	// --- DIRECTIONAL LIGHT ---
	m_pShaderManager->setVec3Value("directionalLight.direction", -0.5f, -1.0f, -0.5f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.1f, 0.1f, 0.12f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	if (g_pCamera != nullptr) {
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position.x, g_pCamera->Position.y, g_pCamera->Position.z);
	}
}

//***************************************************************

void SceneManager::DefineObjectMaterials()
{
	// Material for the Floor 
	OBJECT_MATERIAL material; 
	material.tag = "pine";
	// Lowering these to 0.6f acts like a "dark wood stain" that won't glow white
	material.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	// Lower specular to 0.1f so it doesn't look wet/plastic
	material.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	material.shininess = 32.0f; // Soften the glint
	m_objectMaterials.push_back(material);

	// --- Material for the rug
	material.tag = "rug";
	material.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	material.specularColor = glm::vec3(0.1f, 0.1f, 0.1f); 
	material.shininess = 2.0f;
	m_objectMaterials.push_back(material);

	// --- Material for the fireplace wall
	material.tag = "ceramic";
	material.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	material.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	material.shininess = 64.0f;
	m_objectMaterials.push_back(material);

	// --- Material for Wood/Complex Objects ---
	material.tag = "wood";
	material.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	material.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	material.shininess = 8.0f;
	m_objectMaterials.push_back(material);

	// --- Material for the leather table (chaged to gold)
	material.tag = "leather";
	material.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	material.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	material.shininess = 20.0f;
	m_objectMaterials.push_back(material);

	
	material.tag = "fabric";
	// Keep diffuse at 1.0 to let the texture colors show clearly
	material.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	// VERY LOW specular (0.01) ensures the curtains look like cloth, not plastic
	material.specularColor = glm::vec3(0.01f, 0.01f, 0.01f);
	// A low shininess (1.0) spreads the light out softly across the wrinkles
	material.shininess = 3.0f;
	m_objectMaterials.push_back(material);

}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/

void SceneManager::RenderScene()
{
	if (m_pViewManager == nullptr) return;

// ==========================================================
// BACKGROUND WALL 
// ==========================================================


	SetShaderColor(0.76f, 0.70f, 0.60f, 1.0f); // Warm Taupe

	glm::vec3 bgWallScale(20.0f, 1.0f, 14.5f);
	SetTransformations(bgWallScale, 90.0f, 0.0f, 0.0f, glm::vec3(0.0f, 5.0f, -10.0f));
	m_basicMeshes->DrawPlaneMesh();

// ==========================================================
//  FLOOR
// ==========================================================

	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	m_pShaderManager->setVec2Value(g_UVScaleName, glm::vec2(4.0f, 3.0f));

	SetShaderMaterial("pine");

	// BIND PINE TEXTURE
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetTextureID()); // Your Pine ID
	m_pShaderManager->setIntValue(g_TextureValueName, 0);

	// DIMENSIONS & POSITION (Standard floor setup)
	glm::vec3 scaleXYZ(20.0f, 1.0f, 15.0f);
	glm::vec3 positionXYZ(0.0f, 0.0f, 0.0f);

	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawPlaneMesh();

	// Cleanup: Reset UV scale before moving to the next object
	m_pShaderManager->setVec2Value(g_UVScaleName, glm::vec2(1.0f, 1.0f));

// ==========================================================
// Center Ceramic wall
// ==========================================================

    // apply material
	SetShaderMaterial("ceramic");

	// Enable textures and bind Ceramic
	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetFloorTextureID());
	m_pShaderManager->setIntValue(g_TextureValueName, 0);

	// Set Tiling
	m_pShaderManager->setVec2Value("uvScale", glm::vec2(2.0f, 4.0f));

	// Main Middle Wall 
	glm::vec3 middleWallScale(16.0f, 29.0f, 4.0f);
	glm::vec3 middleWallPosition(0.0f, 5.0f, -9.5f);

	SetTransformations(middleWallScale, 0.0f, 0.0f, 0.0f, middleWallPosition);
	m_basicMeshes->DrawBoxMesh();


	// Side Wall Thickness & Scale
    float sideWallWidth = 0.1f;
	glm::vec3 sideWallScale(sideWallWidth, middleWallScale.y, middleWallScale.z);

	// Left Side Wall 
	// Positions the side wall precisely at the left edge of the main wall
	glm::vec3 leftSideWallPos(
		middleWallPosition.x - (middleWallScale.x / 2.0f) - (sideWallWidth / 2.0f),
		middleWallPosition.y,
		middleWallPosition.z);

	SetTransformations(sideWallScale, 0.0f, 0.0f, 0.0f, leftSideWallPos);
	m_basicMeshes->DrawBoxMesh();

	// Right Side Wall
	// Positions the side wall precisely at the right edge
	glm::vec3 rightSideWallPos(
		middleWallPosition.x + (middleWallScale.x / 2.0f) + (sideWallWidth / 2.0f),
		middleWallPosition.y,
		middleWallPosition.z);

	SetTransformations(sideWallScale, 0.0f, 0.0f, 0.0f, rightSideWallPos);
	m_basicMeshes->DrawBoxMesh();

	// Cleanup for next objects 
	// Resetting uvScale to standard 1:1 ratio
	m_pShaderManager->setVec2Value("uvScale", glm::vec2(1.0f, 1.0f));

	// ==========================================================
	// THE MANTEL
	// ==========================================================

	SetShaderMaterial("wood");
	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetWhiteWoodID());
	m_pShaderManager->setIntValue(g_TextureValueName, 0);

	//  POSITIONING
	glm::vec3 mantelPosition(0.0f, 8.0f, -7.3f);

	//  THE MAIN MANTEL (midle)
	glm::vec3 mantelScale(13.5f, 0.8f, 2.3f);
	SetTransformations(mantelScale, 0.0f, 0.0f, 0.0f, mantelPosition);
	m_basicMeshes->DrawBoxMesh();

	//  THE CROWN EDGE (Top detail)
	glm::vec3 crownScale(14.5f, 0.4f, 2.5f); // top
	glm::vec3 crownPos = mantelPosition + glm::vec3(0.0f, 0.6f, 0.1f); // Layered forward

	SetTransformations(crownScale, 0.0f, 0.0f, 0.0f, crownPos);
	m_basicMeshes->DrawBoxMesh();

	// THE BASE MOLDING (Bottom detail)
	glm::vec3 baseCapScale(12.5f, 0.2f, 2.0f);
	glm::vec3 baseCapPos = mantelPosition + glm::vec3(0.0f, -0.50f, 0.0f);

	SetTransformations(baseCapScale, 0.0f, 0.0f, 0.0f, baseCapPos);
	m_basicMeshes->DrawBoxMesh();

	// ==========================================================
	// Candles and holders 
	// ==========================================================

	m_pShaderManager->use();
	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	m_pShaderManager->setBoolValue("bUseLighting", true);
	SetShaderMaterial("metal");

	// Bind Gold Texture (using WhiteWood ID + Gold Color Tint)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetWhiteWoodID());
	m_pShaderManager->setIntValue(g_TextureValueName, 0);
	SetShaderColor(1.0f, 0.85f, 0.3f, 1.0f); // Professional Gold Tint

	// COORDINATES: 
	// Mantel Top is at Y=8.81f. Mantel Front is at Z=-6.8f.
	float surfaceY = 8.81f;
	float shelfZ = -6.8f;

	// Define the two identical positions (Left and Right)
	glm::vec3 candlePositions[] = {
		glm::vec3(-5.0f, surfaceY, shelfZ),
		glm::vec3(5.0f, surfaceY, shelfZ)
	};

	for (const auto& pos : candlePositions)
	{
		// --- THE BASE (Box) ---
		// Height 0.2f. Center is at 0.1f above surface.
		SetTransformations(glm::vec3(1.0f, 0.6f, 1.0f), 0.0f, 0.0f, 0.0f, pos + glm::vec3(.0f, 0.1f, 0.0f));
		m_basicMeshes->DrawBoxMesh();

		// --- THE STEM (Cylinder) ---
		// Height 1.0f. Center is at 0.2f (Base) + 0.5f (Half Stem) = 0.7f
		SetTransformations(glm::vec3(0.3f, 1.0f, 0.2f), 0.0f, 0.0f, 0.0f, pos + glm::vec3(0.0f, 0.2f, 0.0f));
		m_basicMeshes->DrawCylinderMesh();

		// --- TOP CUP (Tapered Cylinder) ---
		// Height 0.4f. Center is at 0.2 (Base) + 1.0 (Stem) + 0.2 (Half Top) = 1.4f
		SetTransformations(glm::vec3(0.5f, 0.4f, 0.5f), 0.0f, 0.0f, 0.0f, pos + glm::vec3(0.0f, 1.4f, 0.0f));
		m_basicMeshes->DrawTaperedCylinderMesh();
	}

	// --- THE CANDLES (White Wax Cylinders) ---
	m_pShaderManager->setBoolValue(g_UseTextureName, false);
	SetShaderMaterial("clay"); // Wax finish
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f); // Pure White

	for (const auto& pos : candlePositions)
	{
		// Candle Height 1.5f. 
		// Center is at 0.2 (Base) + 1.0 (Stem) + 0.4 (Top) + 0.75 (Half Candle) = 2.35f
		SetTransformations(glm::vec3(0.35f, 0.8f, 0.35f), 0.0f, 0.0f, 0.0f, pos + glm::vec3(0.0f, 1.0f, 0.0f));
		m_basicMeshes->DrawCylinderMesh();
	}

	m_pShaderManager->setBoolValue(g_UseTextureName, true);

	// ==========================================================
	// Curtains and Rods
	// ==========================================================
	
	float cZ = -9.2f;   // The Front Bar depth
	float cY = 18.6f;   // The Height

	m_pShaderManager->use(); // Ensure shader is active first 

	// FABRIC TEXTURE (Slot 0)
	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	SetShaderMaterial("fabric");

	//  Activate Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	//  Bind the specific Fabric ID (Bypassing the tag system)
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetFabricID());
	// Tell the shader's 'objectTexture' to look at Unit 0
	m_pShaderManager->setIntValue(g_TextureValueName, 0);

	// WRINKLE OVERLAY (Slot 1)
	m_pShaderManager->setBoolValue("bUseTextureOverlay", true);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetFabricID());
	m_pShaderManager->setIntValue("overlayTexture", 1); // Tells 'overlayTexture' to look at Unit 1

	// WRINKLE SCALE
	m_pShaderManager->setVec2Value(g_UVScaleName, glm::vec2(8.0f, 1.0f));

	// DRAW 
	SetTransformations(glm::vec3(4.5f, 18.6f, 0.1f), 0.0f, 0.0f, 0.0f, glm::vec3(-17.0f, 9.3f, -9.2f));
	m_basicMeshes->DrawBoxMesh();
	SetTransformations(glm::vec3(4.5f, 18.6f, 0.1f), 0.0f, 0.0f, 0.0f, glm::vec3(17.0f, 9.3f, -9.2f));
	m_basicMeshes->DrawBoxMesh();

	// --- CLEANUP ---
	m_pShaderManager->setBoolValue("bUseTextureOverlay", false);
	m_pShaderManager->setVec2Value(g_UVScaleName, glm::vec2(1.0f, 1.0f));


	// HARDWARE (Rods, Plugs, and Joints)
	m_pShaderManager->setBoolValue(g_UseTextureName, false);
	SetShaderMaterial("metal");
	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f);

	// --- SHARED SETTINGS ---
	float rT = 0.15f; // Rod Thickness
	float rJ = 0.3f;  // Joint Size (Large to hide seams)
	float plugLen = 0.82f; // Distance from Bar to Wall 
	float plugZ = -9.9f;   // The exact midpoint between bar and wall

	// --- LEFT WINDOW ---
	// Sphere Joints 
	SetTransformations(glm::vec3(rJ), 0.0f, 0.0f, 0.0f, glm::vec3(-14.0f, cY, cZ));
	m_basicMeshes->DrawSphereMesh();
	SetTransformations(glm::vec3(rJ), 0.0f, 0.0f, 0.0f, glm::vec3(-20.0f, cY, cZ));
	m_basicMeshes->DrawSphereMesh();

	// Horizontal Bar
	SetTransformations(glm::vec3(rT, 6.1f, rT), 0.0f, 0.0f, 90.0f, glm::vec3(-14.0f, cY, cZ));
	m_basicMeshes->DrawCylinderMesh();

	// Wall Plugs
	SetTransformations(glm::vec3(rT, plugLen, rT), 90.0f, 0.0f, 0.0f, glm::vec3(-14.0f, cY, plugZ));
	m_basicMeshes->DrawCylinderMesh();
	SetTransformations(glm::vec3(rT, plugLen, rT), 90.0f, 0.0f, 0.0f, glm::vec3(-20.0f, cY, plugZ));
	m_basicMeshes->DrawCylinderMesh();


	// --- RIGHT WINDOW ---
	// Sphere Joints
	SetTransformations(glm::vec3(rJ), 0.0f, 0.0f, 0.0f, glm::vec3(14.0f, cY, cZ));
	m_basicMeshes->DrawSphereMesh();
	SetTransformations(glm::vec3(rJ), 0.0f, 0.0f, 0.0f, glm::vec3(20.0f, cY, cZ));
	m_basicMeshes->DrawSphereMesh();

	// Horizontal Bar
	SetTransformations(glm::vec3(rT, 6.1f, rT), 0.0f, 0.0f, 90.0f, glm::vec3(20.0f, cY, cZ));
	m_basicMeshes->DrawCylinderMesh();

	// Wall Plugs
	SetTransformations(glm::vec3(rT, plugLen, rT), 90.0f, 0.0f, 0.0f, glm::vec3(14.0f, cY, plugZ));
	m_basicMeshes->DrawCylinderMesh();
	SetTransformations(glm::vec3(rT, plugLen, rT), 90.0f, 0.0f, 0.0f, glm::vec3(20.0f, cY, plugZ));
	m_basicMeshes->DrawCylinderMesh();

	m_pShaderManager->setBoolValue(g_UseTextureName, true);


	
// ==========================================================
// WINDOW ASSEMBLY (Reflective Glass + Frames)
// ==========================================================

// DIMENSIONS & POSITIONING
	glm::vec3 windowScale(6.0f, 11.0f, 0.2f); // Fixed to positive 13.0f
	float windowZ = -9.8f;
	glm::vec3 windowPositions[] = {
		glm::vec3(-14.0f, 12.0f, windowZ),
		glm::vec3(14.0f, 12.0f, windowZ)
	};

	// DEFINE THE DRAWING LOGIC
	auto DrawWindowFrame = [&](const glm::vec3& center)
		{
			// Frame Z: Slightly in front of glass to avoid Z-fighting
			glm::vec3 frameOffset(0.0f, 0.0f, 0.15f);

			// Top & Bottom 
			SetTransformations(glm::vec3(windowScale.x + 0.6f, 0.8f, 0.5f), 0.0f, 0.0f, 0.0f, center + glm::vec3(0.0f, (windowScale.y / 2), 0.0f) + frameOffset);
			m_basicMeshes->DrawBoxMesh();
			SetTransformations(glm::vec3(windowScale.x + 0.6f, 0.8f, 0.5f), 0.0f, 0.0f, 0.0f, center - glm::vec3(0.0f, (windowScale.y / 2), 0.0f) + frameOffset);
			m_basicMeshes->DrawBoxMesh();

			// Sides 
			SetTransformations(glm::vec3(0.6f, windowScale.y, 0.5f), 0.0f, 0.0f, 0.0f, center - glm::vec3((windowScale.x / 2), 0.0f, 0.0f) + frameOffset);
			m_basicMeshes->DrawBoxMesh();
			SetTransformations(glm::vec3(0.6f, windowScale.y, 0.5f), 0.0f, 0.0f, 0.0f, center + glm::vec3((windowScale.x / 2), 0.0f, 0.0f) + frameOffset);
			m_basicMeshes->DrawBoxMesh();

			// Center Dividers 
			SetTransformations(glm::vec3(0.1f, windowScale.y, 0.1f), 0.0f, 0.0f, 0.0f, center + frameOffset);
			m_basicMeshes->DrawBoxMesh();
			SetTransformations(glm::vec3(windowScale.x, 0.4f, 0.4f), 0.0f, 0.0f, 0.0f, center + frameOffset);
			m_basicMeshes->DrawBoxMesh();
		};

	// --- DRAW GLASS ---
	SetShaderMaterial("glass");
	m_pShaderManager->setFloatValue("material.shininess", 128.0f);
	SetShaderColor(0.5f, 0.7f, 1.0f, 0.4f);
	for (auto& pos : windowPositions) {
		SetTransformations(windowScale, 0.0f, 0.0f, 0.0f, pos);
		m_basicMeshes->DrawBoxMesh();
	}

	// --- DRAW FRAMES ---
	SetShaderMaterial("wood");
	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetWhiteWoodID());
	m_pShaderManager->setIntValue(g_TextureValueName, 0);

	for (auto& pos : windowPositions) {
		DrawWindowFrame(pos); // Now the identifier is known!
	}

	

// ==========================================================
// THE RUG 
// ==========================================================

	m_pShaderManager->setBoolValue("bUseTexture", true);
	m_pShaderManager->setBoolValue("bUseTextureOverlay", true);

	
	m_pShaderManager->setVec2Value("uvScale", glm::vec2(10.0f, 10.0f));

	// SLOT 0: Main Rug Pattern
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetRugTextureID());
	m_pShaderManager->setIntValue("objectTexture", 0);

	// SLOT 1: The Same Pattern (
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetRugTextureID());
	m_pShaderManager->setIntValue("overlayTexture", 0);

	// RENDER (Sitting at 0.02f to clear the floor)
	SetTransformations(glm::vec3(12.0f, 1.0f, 8.0f), 0.0f, 0.0f, 0.0f, glm::vec3(0.0f, 0.02f, 2.0f));
	m_basicMeshes->DrawPlaneMesh();

	// CLEANUP 
	m_pShaderManager->setBoolValue("bUseTextureOverlay", false);
	m_pShaderManager->setVec2Value("uvScale", glm::vec2(1.0f, 1.0f));



	// ==========================================================
    // OVAL TABLE 
    // ==========================================================

     // Table Base
	glm::vec3 tableBaseScale(3.5f, 1.5f, 2.0f);
	glm::vec3 tableBasePos(0.0f, 0.20f, 0.0f); // Positioned on the rug

	
	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	m_pShaderManager->setVec2Value("uvScale", glm::vec2(1.0f, 1.0f)); // Standard mapping
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetLeatherID());
	m_pShaderManager->setIntValue(g_TextureValueName, 0);

	SetTransformations(tableBaseScale, 0.0f, 0.0f, 0.0f, tableBasePos);
	m_basicMeshes->DrawCylinderMesh();

	// --- Glass Table Top ) ---
	glm::vec3 glassTopScale(tableBaseScale.x + 0.4f, 0.1f, tableBaseScale.z + 0.4f);
	glm::vec3 glassTopPos(
		tableBasePos.x,
		tableBasePos.y + (tableBaseScale.y / 2.0f) + (glassTopScale.y / 2.0f),
		tableBasePos.z);

	// Disable textures to use the semi-transparent color for glass
	m_pShaderManager->setBoolValue(g_UseTextureName, false);
	SetShaderColor(0.5f, 0.75f, 0.8f, 0.4f); 

	SetTransformations(glassTopScale, 0.0f, 0.0f, 0.0f, glassTopPos);
	m_basicMeshes->DrawBoxMesh();

	// Cleanup:
	m_pShaderManager->setBoolValue(g_UseTextureName, true);


	// ==========================================================
	// BASEBOARD ASSEMBLY (White Wood Texture)
	// ==========================================================

	// material
	SetShaderMaterial("wood");
	m_pShaderManager->setBoolValue(g_UseTextureName, true);
	m_pShaderManager->setVec2Value(g_UVScaleName, glm::vec2(1.0f, 1.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pViewManager->GetWhiteWoodID());
	m_pShaderManager->setIntValue(g_TextureValueName, 0);

	float bgBaseboardTotalHeight = 1.3f;
	float bgBaseboardTotalDepth = 0.4f;
	float bgWallWidth = 40.0f;
	float bgWallZ = -10.0f;

	// Main flat part of the baseboard (the "base")
	glm::vec3 baseScale(bgWallWidth, bgBaseboardTotalHeight * 0.8f, bgBaseboardTotalDepth);
	glm::vec3 basePos(0.0f, baseScale.y / 2.0f, bgWallZ + (bgBaseboardTotalDepth / 2.0f));

	SetTransformations(baseScale, 0.0f, 0.0f, 0.0f, basePos);
	m_basicMeshes->DrawBoxMesh();

	// Thinner upper "edge" detail
	float edgeHeight = bgBaseboardTotalHeight * 0.2f;
	float edgeDepth = bgBaseboardTotalDepth * 0.8f;
	glm::vec3 edgeScale(bgWallWidth, edgeHeight, edgeDepth);
	glm::vec3 edgePos(0.0f, baseScale.y + (edgeHeight / 2.0f), bgWallZ + (edgeDepth / 2.0f) + 0.01f);

	SetTransformations(edgeScale, 0.0f, 0.0f, 0.0f, edgePos);
	m_basicMeshes->DrawBoxMesh();

	// Reset for next objects
	m_pShaderManager->setBoolValue(g_UseTextureName, true);

	// ==========================================================
    //  TV (also can be use as a picture frame with wallpaper)
    // ==========================================================

	m_pShaderManager->setBoolValue(g_UseTextureName, false);
	SetShaderColor(0.05f, 0.05f, 0.05f, 1.0f); // Black screen (no wallpaper added)

	glm::vec3 tvScale(13.0f, 7.0f, 0.2f);

	float tvPosY = middleWallPosition.y + (middleWallScale.y / 2.0f) - (tvScale.y / 2.0f) - 2.0f;
	float tvPosZ = middleWallPosition.z + 2.0f;
	glm::vec3 tvPosition(0.0f, tvPosY, tvPosZ);

	SetTransformations(tvScale, 0.0f, 0.0f, 0.0f, tvPosition);
	m_basicMeshes->DrawBoxMesh();

	
	//=======================================//
	//    Ceiling 
	//=======================================//

	// DIMENSIONS
	glm::vec3 ceilingScale(40.0f, 0.4f, 38.0f);
	float ceilingBottomY = 19.5f; // We want the visible surface at 19.5

	// POSITION: Center = Bottom + (Half Height)

	glm::vec3 ceilingPosition(0.0f, ceilingBottomY + (ceilingScale.y / 2.0f), 0.0f);

	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f); // Solid white

	SetTransformations(ceilingScale, 0.0f, 0.0f, 0.0f, ceilingPosition);
	m_basicMeshes->DrawBoxMesh();


	//======================================//
	//    Recessed Rounded Lights
	//======================================//

	// LIGHT SCALE (0.3f height)
	glm::vec3 lightScale(1.0f, 0.3f, 1.0f);

	// INSET POSITIONING: 
	// Setting Y to 19.65f buries 0.15f of the light inside the box.
	float insetLightY = ceilingBottomY - 0.10f;

	float roomX = 10.0f;
	float roomZ = 5.0f;

	std::vector<glm::vec3> lightPositions = {
		glm::vec3(-roomX + 3.0f, insetLightY, -roomZ + 3.0f),
		glm::vec3(roomX - 3.0f,  insetLightY, -roomZ + 3.0f),
	};

	//  Render Loop
	for (const auto& pos : lightPositions)
	{
		// Disable lighting for the "bulb" glow
		m_pShaderManager->setBoolValue(g_UseLightingName, false);
		// Cool Daylight White (Crisp light)
		SetShaderColor(0.9f, 0.95f, 1.0f, 1.0f);

		SetTransformations(lightScale, 0.0f, 0.0f, 0.0f, pos);
		m_basicMeshes->DrawCylinderMesh();

		// Turn lighting back ON for the room
		m_pShaderManager->setBoolValue(g_UseLightingName, true);
	}
	

// ==========================================================
// DIGITAL FIREPLACE 
// ==========================================================

// DIMENSIONS
glm::vec3 fpBodyScale(12.0f, 4.0f, 0.4f); 

// POSITIONING
float fpBodyPosY = 4.25f;
float fpBodyPosZ = -7.3f; 

glm::vec3 fpBodyPos(0.0f, fpBodyPosY, fpBodyPosZ);

// --- DRAW FIREPLACE BODY ---
m_pShaderManager->setBoolValue(g_UseTextureName, false);
SetShaderColor(0.15f, 0.15f, 0.15f, 1.0f); // Dark Charcoal
SetTransformations(fpBodyScale, 0.0f, 0.0f, 0.0f, fpBodyPos);
m_basicMeshes->DrawBoxMesh();

// --- DRAW GLASS PANEL ---
glm::vec3 glassScale(fpBodyScale.x - 1.0f, fpBodyScale.y - 0.5f, 0.05f);

// Position glass 0.02f in front of the fireplace body
glm::vec3 glassPos(fpBodyPos.x, fpBodyPos.y, fpBodyPos.z + (fpBodyScale.z / 2.0f) + 0.02f);

SetShaderColor(0.5f, 0.75f, 0.8f, 0.35f); // Light blue glass
SetTransformations(glassScale, 0.0f, 0.0f, 0.0f, glassPos);
m_basicMeshes->DrawBoxMesh();


	}
	



