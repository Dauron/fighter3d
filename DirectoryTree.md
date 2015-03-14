# Introduction #

SVN repository consists of three root direcotries:
  * CamFighter - main development branch
  * Doc         - documentation and concept art branch
  * branches    - major releases

# Project Directory Tree #

Main development branch tree looks as follows:

| **Directory**      | **Description**                                      |
|:-------------------|:-----------------------------------------------------|
| \ App Framework  | application skeleton, IWindow, base scene          |
| \ \ Input        | keyboard and mouse manager                         |
| \ \ OGL          | OpenGL implementation of the IWindow               |
| \ Data           | game data and configuration files                  |
| \ \ models       | models, animations, maps                           |
| \ \ shaders      | GLSlang shaders                                    |
| \ \ textures     | model textures                                     |
| \ Graphics       | graphics engine                                    |
| \ \ Textures     | texture management                                 |
| \ \ OGL          | OpenGL specific code, map rendering                |
| \ \ \ Extensions | OpenGL extensions                                  |
| \ \ \ Render     | xModel object rendering                            |
| \ Math           | mathematics library                                |
| \ \ Cameras      | cameras, field of view                             |
| \ \ Figures      | 3D figures used by BVH                             |
| \ \ Tracking     | object tracking                                    |
| \ Models         | 3D models manager                                  |
| \ \ lib3ds       | lib3ds copy used by Visual Studio project          |
| \ \ lib3dx       | native for this project library of xModel objects  |
| \ MotionCapture | entry point for the motion capture library         |
| \ Multiplayer    | entry point for the network comunication           |
| \ Physics        | physics engine, base physical objects              |
| \ \ Colliders    | BVH collision detection                            |
| \ \ Verlet       | Verlet integration and constraints                 |
| \ Source Files   | game logic, IScene implementations                 |
| \ \ MenuStates  | screens for menu scene                             |
| \ Utils          | templates and auxiliary tools                      |
| \ VisualStudio  | Visual Studio 2005 solution and project            |
| \ World          | specific implementation of the object used in game |