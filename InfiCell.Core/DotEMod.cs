using Harpoon.Core;
using System;

namespace InfiCell.Core
{
    public class DotEMod : Mod
    {

        public override ModMetadata Metadata =>
            new ModMetadata
            {
                AuthorName = "Nielsbishere",
                ModDescription = "A new beginning to a wonderful mod!",
                ModName = "InfiCell.Core",
                ModVersion = "0.0.0"
            };

        public override void Initialize()
        {
            Dungeon dungeon = SingletonManager.Get<Dungeon>();
            dungeon.AddDust(40);
        }
    }
}
