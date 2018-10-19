
namespace InfiCell.Core
{
    public interface IDungeonListener : IListener
    {

        void OnEnter(Dungeon dungeon); //TODO: bool fromSave and isMultiplayer
        void OnLeave(Dungeon dungeon);

        //When a dungeon is won; if isLastLevel is true, you've beaten the game
        void OnWin(Dungeon dungeon, bool isLastLevel);

    }
}
