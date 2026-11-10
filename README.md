# OpenGL 3D Scene Renderer

An interactive 3D living-room scene rendered in **C++ and modern OpenGL**. The scene is modeled from a 2D reference photo using only low-polygon primitive shapes (cylinders, boxes, planes, and tapered shapes), then given textures, lighting, and a free-moving camera.

<!-- Add a screenshot: upload it to this repo as screenshot.png, then remove the comment marks around the next line -->
<!-- ![Scene screenshot](screenshot.png) -->

## What it demonstrates

- **Complex objects from primitives:** objects such as the fireplace mantel are assembled from several meshes inside reusable functions, so scale and position can be adjusted with a single call
- **Multi-texturing:** the rug and curtains blend two texture units sampling the same image at different frequencies, which adds surface detail without extra texture files
- **Phong lighting:** ambient, diffuse, and specular lighting with a directional light, a colored point light with attenuation (the fireplace glow), and a spotlight, all in one shader pipeline
- **Camera control:** a free-fly camera with mouse look and scroll, plus a switch between perspective and orthographic projection
- **Modular code:** separate classes for scene and asset management (`SceneManager`) and for the window, camera, and input (`ViewManager`)

## Controls

| Key | Action |
|---|---|
| W / S | Move forward / backward |
| A / D | Move left / right |
| Q / E | Move down / up |
| Mouse | Look around |
| Scroll wheel | Adjust camera speed |
| P / O | Perspective / orthographic projection |
| Esc | Quit |

## Tech stack

C++, OpenGL, GLFW, GLEW, GLM, stb_image. Built with Visual Studio on Windows.

## Project layout

- `Source/`: scene manager, view manager, and the main program
- `textures/`: texture images used by the scene
- [`DESIGN_NOTES.md`](DESIGN_NOTES.md): the design decisions behind the scene

## Background

Final project for CS-330 (Computer Graphic Design) at SNHU.
