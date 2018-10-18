using Harpoon.Core;
using System;
using System.Collections.Generic;

namespace InfiCell.Core
{
    public class InfiCellMod : Mod
    {

        //Per instance stuff

        public InfiCellMod()
        {
            instances.Add(this);
        }

        public virtual void OnDungeonLoad(Dungeon dungeon) { }
        public virtual void OnDungeonQuit(Dungeon dungeon) { }
        public virtual void OnMenuLoad() { }

        //Static stuff like helpers

        private static List<InfiCellMod> instances = new List<InfiCellMod>();

        private static void NotifyDungeonLoad(Dungeon dungeon)
        {
            foreach (InfiCellMod mod in instances)
                mod.OnDungeonLoad(dungeon);
        }

        private static void NotifyDungeonQuit(Dungeon dungeon)
        {
            foreach (InfiCellMod mod in instances)
                mod.OnDungeonQuit(dungeon);
        }

    }
}
