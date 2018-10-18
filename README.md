# InfiCell
InfiCell is a reflective mono DLL injector for Dungeon of the Endless that doesn't modify any of the source files or includes them. It uses a minified version of [Harpoon](https://github.com/nielsbishere/harpoon) and uses it to attach to or launch a new game.  
  
InfiCell allows you to modify common features, like modules, rooms, NPCs, monsters and so on. This doesn't require any knowledge about how DotE's source code is structured. However, features that aren't supported by InfiCell can still be accessed through C# reflection.  
  
InfiCell's Listeners allow you to listen to events happening in the dungeon; such as rooms opening, crystal being carried, monsters spawning, rooms spawning, entering dungeons, leaving dungeons, etc. This can prove very useful when you want to add custom game modes, custom events and so on. However, do note that you should implement networking correctly when doing so. [Harmony](https://github.com/pardeike/Harmony) is primarily used for the listeners.

## APIs used
[Harpoon](https://github.com/nielsbishere/harpoon) for reflective DLL mono injection.  
[Harmony](https://github.com/pardeike/Harmony) for C# bytecode injection.
