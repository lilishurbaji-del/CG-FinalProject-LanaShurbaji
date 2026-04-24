# Game Description
This project is a simple 3D escape-style game developed using OpenGL.
The player moves through a forest environment with obstacles such as trees and moving walls, aiming to reach a house at the end of the path.
To win the game, the player must first collect a key placed along the path, which is required to unlock the final house.
The game includes basic collision detection, camera movement, fog effect, and a simple HUD that shows progress and key status.
The main goal is to demonstrate core computer graphics concepts such as rendering pipelines, transformations (Model/View/Projection), and real-time interaction.
# Controls
Keyboard:
Arrow Up → Move forward
Arrow Down → Move backward
Arrow Right → Move right
Arrow Left → Move left
Mouse:
Mouse movement controls the camera direction (look around)
# Build Instructions
To compile and run the project, follow these steps:
1. Requirements
Visual Studio (C++ Development Tools)
OpenGL libraries:
GLFW
GLEW
GLM
2. Setup
Open the project in Visual Studio.
Ensure all source files are included in the project.
Make sure shader and asset folders (if used) are in the correct directory.
3. Linking Libraries
Link the required libraries in the project settings:
glfw3.lib
glew32s.lib
opengl32.lib
Also, add include and library directories for GLFW, GLEW, and GLM.
4. Run the Project
Set the project as Startup Project.
Run using Local Windows Debugger.
