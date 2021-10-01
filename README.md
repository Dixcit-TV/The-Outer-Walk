# The-Outer-Walk
3D DirectX11 based game built around procedural planet generation.

Unfortunately I cannot share the whole project as the engine it was built on (Overlord Engine) is a Digital Art and Entertainment owned engine.
Although I might be harder to follow the code flow, I am sharing my the sources that I personnally implemented.

About the implementation of the project itself: 
The Engine works with different scenes that can be added to the scene major when the application start and the scenes can be toggled at run-time. Each scene has a set of function they can override (on initilize, on activation, on deactivation) that can be use to load and unload resources and assets. The way I built my game is by creating a scene per menu and another one for the actual "game". I then switch from a scene to another on button clicks or key pressed. I then have a "PersistentDatamanager" used to collect data and query data that should live through a scene change (ie. the current planet data when switching to in-game/editor menu).

The Engine sources I am sharing here are the ones files I fully implemented throughout the different exercises and labs I had during my graphics 2 class, or that I highly edited to fit my fit my requirements when building my planet generator.

Aside from these, I've worked on the Text and Sprite renderer, the setup of the Controller component to use Physx character controller, modify the scene logic to order and call the different draw calls (skybox, meshes, shadows, text, sprites, post processing), added caching for sound effect in the FMod sound manager.

[![The Outer Walk - Trailer](https://yt-embed.herokuapp.com/embed?v=TT7pnFtVSV8)](https://www.youtube.com/watch?v=TT7pnFtVSV8)

## Introduction
For Graphics Programming 2, after a semester of learning and implementing environment & normal mapping, phong & diffuse shading, shadow mapping, animation, particles, and basics of DX11 and the Physx API, we are tasked to create a vertical slice of a pre-existing or totally invented game using the DAE provided Overlord Engine. I then decided to come up with my own idea: The Outer Walk.

Over implementing a proper game level (like Bomberman, Overcooked-like, Prince of persia, etc...) with a begining and end, I wanted to focus on shaders and working with the GPU to learn by practicing. I am a big fan of procedural generation and I recently came accross a videos and resources about procedural planet generation that seemed like a really interesting challenge. The final result being a real-time planet generator using an IMGUI menu to control the generation parameters.

## The sphere generation
Many ways can be found online to generate a sphere triangle mesh procedurally with variable tesselation; UV-sphere, Cube-sphere, Ico-sphere. I had to pick one for my endeavor and the Ico-spehere seemed like a pretty good candidate as the base Icosahedron mesh allows for a (relatively) even distribution of triangles after tesselation, triangles that are (almost) equilateral.
Now that I had a starting shape, I needed to decide on a tesselation "algorithm". Considering 4 other courses on the side and a little over a month left, I decided to kill 2 birds (lack of time + new knowledge) with one stone and dive into the tesselation shader stage available in DX11 pipeline.
DX11 tesselation stage allows to define a the edge and inner subdivision per triangle, I found out that subdividing the edge and inner triangle by the same number result in the "even distribution" I was looking for. From there, what I needed was to bind my tesselation vertex list ouput to a vertex buffer to use for rendering in a future pass. One draw-back I discovered from using the tesselation shader was the tesselation level limited to 64 subdisions, but it was definetly good enough for my experimentation.

## The landscape generation
I had my sphere, the next step was to generate the landmass and oceans. A very common way to do so is by using noise and more specifically Perlin and Simplex noise due to their "smooth gradient" nature. Using a HLSL implementation of a simplex noise function, I created several noise layers that would each generate a different aspect of my planet terrain; a layer for the main land shape, one for the overal detailing and a final one for the mountains. Each layers are then added on top of each other to form the final planet. All this being done in the domain shader of our tesselation stage.

## Shading & shadows & ocean
The final step was to add some shading to this grey planet. For this I stored each vertices height and steepness. I the used those values to blend shore, land and mountains colors together and try to give the planet a natural look. Shadows were added using shadow mapping. And as a final touch, followed the idea to create my "oceans" shading as a post processing effect that is traced on top of the final planet mesh, with waves added with triplanar normal mapping.

## The planet physics and collider
While looking into how to generate a collider using the Physics API, so my character could walk on it, I discovered they provide a tool call height fields which would fit perfectly terrains and would provide an efficient result. The problem was that I had no idea how I could use height fields on a spherical terrain. Plan B was then to create a triangleMesh using the vertices I was getting back from the GPU. Using a staging buffer I copied my vertices in a vector that I then fed to the PhysX cooking API. I worked well physics wise, my character was properly stepping around, not clipping through... Performance wise I got a nice drop from 2k to 300fps which I suppose was to be expected but still keeping in mind that I could probably find a more efficient solution.
Next I gave each of my planets a random gravity and rotation, which is fed to the character has it has to abide by it.

Fun note: The Overlord Engine we were provided with came with some of the PhysX API setup but not the PhysX Cooking DLL I needed to generate my planet collider. The only way I found to get that missing DLL was to seek the PhysX 3.4 sources, open it in Visual Studio 19, FIX THE BUNCH OF COMPILATION ERRORS DUE TO A NEWER VS COMPILER VERSION and after a few hours of struggle could finally get what I want.

## Resources
- "3D Game Programming with DirectX11" by Frank D. Luna
- Simplex noise library for HLSL: [Github](https://gist.github.com/fadookie/25adf86ae7e2753d717c)
- Depth to viewSpace: [mynameismjp.wordpress.com](https://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/)
- Coding Adventure: [Procedural Moons and Planets: Youtube](https://www.youtube.com/watch?v=lctXaT9pxA0)
- Triplanar mapping & normal map: [bgolus.medium.com](https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a)
