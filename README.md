# InfiCell
InfiCell is a reflective mono DLL injector for Dungeon of the Endless that doesn't modify any of the source files or includes them. It uses a minified version of https://github.com/nielsbishere/harpoon.

InfiCell's Core API allows you to modify common features, like modules, rooms, NPCs, monsters and so on. This doesn't require any knowledge about how DotE's source code is structured. However, features that aren't supported by InfiCell can still be accessed through C# reflection.

