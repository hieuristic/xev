XEV - Xarlie's Express Visualization
A Graphics Engine for Building Games, Visualizations, and Interactive Arts.

Made by Human for Human.

----------------------------------
Licensed under Apache 2.0
Copyright © Nguyen Minh Hieu, 2026
----------------------------------

1. Installation

You can build the example app with:

```
cmake -B build && cmake --build build -j8 && ./build/bin/xev
```


2. Coordinate System

Local object frame uses right-handed RDF system (+x - Right, +y - Down, +z - Front).


3. Features

 - headless renderering
 - clustered forward renderer
 - bindless descriptor set, dynamic rendering


4. Architecture

The code base is split into system and resource. All systems and their lifetime are
completely owned by the Engine class. The engine class contains a unique_ptr to
uninitialized systems and require explicit initialization via the init_<system_name>()
call. Resources do NOT own their own data and have to be explicitly created
and destroyed either by a system or the application. To get started, please checkout
`app/demo/` where you can find a minimal example on how to draw a gltf scene.
