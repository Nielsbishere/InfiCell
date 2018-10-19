using Harpoon.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Harmony;

namespace InfiCell.Core
{
    public class InfiCellMod : Mod
    {

        //Per instance stuff

        public InfiCellMod()
        {
            instances.Add(this);
            AddListeners(this);
        }

        public void AddListeners<T>(T listener)
        {
            Type[] tlisteners = listener.GetType().FindInterfaces(new TypeFilter(InterfaceFilter), typeof(IListener));

            foreach(Type type in tlisteners)
            {

                if (type == typeof(IListener))
                    continue;

                Console.WriteLine($"Registering {type.FullName} : InfiCell.Core.IListener");
                
                if (!listeners.ContainsKey(type))
                    listeners.Add(type, new List<IListener>());

                List<IListener> values = listeners[type];
                values.Add((IListener)listener);
            }
        }

        //Static stuff like helpers

        public static bool InterfaceFilter(Type typeObj, Object criteriaObj)
        {
            return ((Type)criteriaObj).IsAssignableFrom(typeObj);
        }

        public static List<T> GetListeners<T>() where T : IListener
        {
            Type type = typeof(T);

            if (!listeners.ContainsKey(type))
                return new List<T>();

            List<IListener> list = listeners[type];
            return list.Cast<T>().ToList();
        }

        //Private static variables

        private static List<InfiCellMod> instances = new List<InfiCellMod>();

        private static Dictionary<Type, List<IListener>> listeners = new Dictionary<Type, List<IListener>>();

        //Listener hooks

        private static void NotifyDungeonLoad(Dungeon __instance)
        {
            foreach (IDungeonListener listener in GetListeners<IDungeonListener>())
                listener.OnEnter(__instance);
        }

        private static void NotifyDungeonQuit(Dungeon __instance)
        {
            foreach (IDungeonListener listener in GetListeners<IDungeonListener>())
                listener.OnLeave(__instance);
        }

        private static void NotifyDungeonVictory(Dungeon __instance)
        {
            foreach (IDungeonListener listener in GetListeners<IDungeonListener>())
                listener.OnWin(__instance, __instance.IsLastLevel());
        }
        
        //Initialize hooks

        protected static void InitializeHooks()
        {
            Console.WriteLine("Initializing core...");

            // Create harmony
            HarmonyInstance harmony = HarmonyInstance.Create("InfiCell.API.harmony");

            //Add a post init call for allowing us to update all mods that a dungeon has been loaded

            MethodInfo dungeonInit = typeof(Dungeon).GetMethod("Start", BindingFlags.Instance | BindingFlags.NonPublic);
            MethodInfo dungeonInitPost = typeof(InfiCellMod).GetMethod("NotifyDungeonLoad", BindingFlags.NonPublic | BindingFlags.Static);
            HarmonyMethod dungeonPostLoad = new HarmonyMethod(dungeonInitPost);
            harmony.Patch(dungeonInit, null, dungeonPostLoad);

            //Add a post destroy call for allowing us to update all mods that the dungeon has been quit

            MethodInfo dungeonQuit = typeof(Dungeon).GetMethod("OnDestroy", BindingFlags.Instance | BindingFlags.NonPublic);
            MethodInfo dungeonQuitPost = typeof(InfiCellMod).GetMethod("NotifyDungeonQuit", BindingFlags.NonPublic | BindingFlags.Static);
            HarmonyMethod dungeonPostQuit = new HarmonyMethod(dungeonQuitPost);
            harmony.Patch(dungeonQuit, null, dungeonPostQuit);

            //Add a post level end 

            MethodInfo dungeonVictory = typeof(Dungeon).GetMethod("OnLevelEndDisplay", BindingFlags.Instance | BindingFlags.NonPublic);
            MethodInfo victoryPost = typeof(InfiCellMod).GetMethod("NotifyDungeonVictory", BindingFlags.NonPublic | BindingFlags.Static);
            HarmonyMethod dungeonVictoryPost = new HarmonyMethod(victoryPost);
            harmony.Patch(dungeonVictory, null, dungeonVictoryPost);

            //Patched all functions

            Console.WriteLine("Initialized core");
        }
    }
}
