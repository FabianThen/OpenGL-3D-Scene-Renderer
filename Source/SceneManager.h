///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include "ViewManager.h" // Added inclusion to access ViewManager

#include <string>
#include <vector>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor - UPDATED to accept ViewManager pointer
	SceneManager(ShaderManager* pShaderManager, ViewManager* pViewManager);

	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag = "";
		uint32_t ID = 0; // Initializing to 0 fixes the warning
	};

	struct OBJECT_MATERIAL
	{
		glm::vec3 diffuseColor = glm::vec3(1.0f);
		glm::vec3 specularColor = glm::vec3(1.0f);
		float shininess = 32.0f;
		std::string tag = "";
	};


private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;

	// pointer to view manager object - ADDED for texture access
	ViewManager* m_pViewManager;

	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;

	// total number of loaded textures
	int m_loadedTextures;
	
    // TEXTURE_INFO m_textureIDs[16]; 
	std::vector<TEXTURE_INFO> m_textureIDs;

    // defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);

	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();

	// free the loaded OpenGL textures
	void DestroyGLTextures();

	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);

	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	// set the transformation values 
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ);

	// set the color values into the shader
	void SetShaderColor(
		float redColorValue,
		float greenColorValue,
		float blueColorValue,
		float alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);

	


public:
	void PrepareScene();
	void DefineObjectMaterials();
	void SetupSceneLights();
	void RenderScene();
	void LoadSceneTextures();


};
