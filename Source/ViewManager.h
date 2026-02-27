///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "ShaderManager.h"
#include "camera.h"
#include <string>
#include "GLFW/glfw3.h" 

class ViewManager
{
public:
    // Constructor and Destructor
    ViewManager(ShaderManager* pShaderManager);
    ~ViewManager();

    // GLFW Window and Callback Methods
    static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);
    static void Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset);
    GLFWwindow* CreateDisplayWindow(const char* windowTitle);
    void PrepareSceneView();

    // --- TEXTURE METHODS (PUBLIC) ---
    void LoadSceneTextures();
    unsigned int LoadTextureFromFile(const char* path);

    // Getters for SceneManager to access texture IDs
    unsigned int GetTextureID() const { return m_textureID; }
    unsigned int GetDetailTextureID() const { return m_detailTextureID; }
    unsigned int GetFloorTextureID() const { return m_floorTextureID; }
    unsigned int GetWhiteWoodID() const { return m_whiteWoodID; }
    unsigned int GetLeatherID() const { return m_leatherID; }

private:
    // Pointers to manager objects
    ShaderManager* m_pShaderManager;
    GLFWwindow* m_pWindow;

    // Internal input handling
    void HandleMouseMovement(double xMousePos, double yMousePos);
    void ProcessKeyboardEvents();

    // Texture IDs
    unsigned int m_textureID;        // Main object texture
    unsigned int m_detailTextureID;  // Secondary object texture
    unsigned int m_floorTextureID;   // wood floor
    unsigned int m_whiteWoodID;      //mantel
    unsigned int m_leatherID;        //table

public:
    
    GLuint GetRugTextureID() const { return m_pRugTextureId; }
    GLuint GetRug2TextureID() const { return m_pRug2TextureId; }
    GLuint GetFabricID() const { return m_fabricID; }


private:
    
    GLuint m_pRugTextureId;
    GLuint m_pRug2TextureId;
    GLuint m_fabricID;
};
