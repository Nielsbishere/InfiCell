using Harpoon.Core;
using System;
using System.Reflection;

namespace InfiCell.Core
{
    class InfiCell : InfiCellMod, IDungeonListener
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
        
        public override void Initialize()
        {
            InfiCellMod.InitializeHooks();
        }
        
        public void OnEnter(Dungeon dungeon)
        {
            Console.WriteLine("Hey dungeon");
            dungeon.AddDust(40);
        }

        public void OnLeave(Dungeon dungeon)
        {
            Console.WriteLine("Bye dungeon");
        }

        public void OnWin(Dungeon dungeon, bool isLastLevel)
        {
            Console.WriteLine($"Finished floor; lastFloor = {isLastLevel}");
        }

    }
}
