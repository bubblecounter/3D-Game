# 3D-Game
It is a 3D video game written using OpenGL, glew, glfw, libraries.


## Functionalities:
#### 1.3D viewing and objects
#### 2.User Input 
With keyboard for player movement

With mouse for camera movement in cheat mode
#### 3.Lighting and Smooth Shading
One light source at top front right corner of the scene

Normal vectors for surfaces defined in vertices array

Ambient, Diffuse and Specualr lighting applied on objects
#### 4.Texture Mapping
Used while drawing scene and emojis on cubes

Texture coordinates for surfaces defined in vertices array

## Advanced Computer Graphics Techniques
#### 1.Collision Detection
Implemented using Axis Aligned Bounding Boxes(AABB) algorithm

Used to detect collision between player and other cubes
#### 2. Shadow Mapping
First rendered the scene from light perspective. The depth values stored in a texture. Then rendered the scene as usual with shadow map.
In fragment shader  for each fragment checked whether the depth value in shadow map is smaller than the closest depth value. Do shadowing with these info. Used shadow bias to prevent shadow acne. Set the resolution of shadow as follows WIDTH = 4096, HEIGHT = 4096 to make the shadows smoother.
## Goal
Collecting as many cubes as possible whoose emoji is same as player emoji.

## Points
+5 point for collecting cube with same emoji.

-2 points for collecting cube with different emoji.

## Interactions
WASD for player movement

SPACE for moving up or down

P for pausing the game

R for resuming the game

C for opening Cheat Mode (In cheat mode you can go everywhere using mouse and WASD

V for closing the Cheat Mode

## References:
[AABB implementation](https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection)

[Shadow Mapping](https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping)

[Text Rendering](https://learnopengl.com/In-Practice/Text-Rendering)

[Tutorials](https://learnopengl.com) for most of the other stuff implemented
