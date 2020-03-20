#pragma once

namespace Prismata
{
    namespace Players
    {
        enum { Player_One = 0, Player_Two = 1, Player_Both = 2, Player_None = 3, Size}; 
    }
        
    namespace Phases
    {
        enum { Action, Defense, Breach, Confirm, Swoosh };
    }
    
    // used when parsing card library file, it lists thing by these names
    namespace SupplyAmount
    {
        enum { Unbuyable = 0, Legendary = 1, Rare = 4, Normal = 10, Trinket = 20 };
    }

    // used for converting AI engine moves back to the real game
    namespace ClickTypes
    {   
        enum { BeginSwipe = 2, EndSwipe = 3, Card = 5, Space = 10 };
    }

    namespace SearchMethods
    {
        enum { AlphaBeta, IDAlphaBeta, MiniMax, Size };
    }

    namespace EvaluationMethods
    {
        enum { Playout, WillScore, WillScoreInflation, Size };
    }

}