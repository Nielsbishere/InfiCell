using Harmony;
using Harpoon.Core;
using System;
using System.Reflection;

namespace InfiCell.Core
{
    class InfiCell : InfiCellMod
    {
        
        public override ModMetadata Metadata =>
            new ModMetadata
            {
                IsEnabled = true,
                AuthorName = "Nielsbishere",
                ModDescription = "Core of InfiCell",
                ModName = "InfiCell.Core",
                ModVersion = "0.0.1",
                Priority = 0
            };

        private static void DungeonPostLoad(Dungeon __instance)
        {
            MethodInfo onDungeonLoad = typeof(InfiCellMod).GetMethod("NotifyDungeonLoad", BindingFlags.NonPublic | BindingFlags.Static);
            onDungeonLoad.Invoke(null, new object[] { __instance });
        }

        private static void DungeonPostQuit(Dungeon __instance)
        {
            MethodInfo onDungeonQuit = typeof(InfiCellMod).GetMethod("NotifyDungeonQuit", BindingFlags.NonPublic | BindingFlags.Static);
            onDungeonQuit.Invoke(null, new object[] { __instance });
        }

        public override void Initialize()
        {

            Console.WriteLine("Initializing core...");

           // Create harmony
            HarmonyInstance harmony = HarmonyInstance.Create("InfiCell.API.harmony");

            //Add a post init call for allowing us to update all mods that a dungeon has been loaded

            MethodInfo dungeonInit = typeof(Dungeon).GetMethod("Start", BindingFlags.Instance | BindingFlags.NonPublic);
            MethodInfo dungeonInitPost = typeof(InfiCell).GetMethod("DungeonPostLoad", BindingFlags.NonPublic | BindingFlags.Static);
            HarmonyMethod dungeonPostLoad = new HarmonyMethod(dungeonInitPost);
            harmony.Patch(dungeonInit, null, dungeonPostLoad);

            //Add a post save and quit call for allowing us to update all mods that the dungeon has been quit

            MethodInfo dungeonQuit = typeof(Dungeon).GetMethod("OnDestroy", BindingFlags.Instance | BindingFlags.NonPublic);
            MethodInfo dungeonQuitPost = typeof(InfiCell).GetMethod("DungeonPostQuit", BindingFlags.NonPublic | BindingFlags.Static);
            HarmonyMethod dungeonPostQuit = new HarmonyMethod(dungeonQuitPost);
            harmony.Patch(dungeonQuit, null, dungeonPostQuit);

            //Patched all functions

            Console.WriteLine("Initialized core");
        }

        public override void OnDungeonLoad(Dungeon dungeon)
        {
            Console.WriteLine("Hey dungeon");
            dungeon.AddDust(40);
        }

        public override void OnDungeonQuit(Dungeon dungeon)
        {
            Console.WriteLine("Bye dungeon");
        }


    }
}
