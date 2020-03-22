#include "PartialPlayer_ActionBuy_EngineerHeuristic.h"

using namespace Prismata;

PartialPlayer_ActionBuy_EngineerHeuristic::PartialPlayer_ActionBuy_EngineerHeuristic(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

// this heuristic is based off elyot's rizzoma post which is copied below 
void PartialPlayer_ActionBuy_EngineerHeuristic::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
    
    // if engineer or drone doesn't exist or isn't buyable then exit
    if (!CardTypes::CardTypeExists("Engineer") || !CardTypes::CardTypeExists("Drone"))
    {
        return;
    }

    const CardType engType = CardTypes::GetCardType("Engineer");
    const CardType droneType = CardTypes::GetCardType("Drone");
    const Action buyEngineer(_playerID, ActionTypes::BUY, engType.getID());
    
    // if we can't buy an engineer then exit
    if (!state.isLegal(buyEngineer))
    {
        return;
    }
    
    // if we have anything other than a drone or an engineer then don't bother with this
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const CardType & type = state.getCardByID(cardID).getType();

        if (type != droneType && type != engType)
        {
            return;
        }
    }

    // if we don't have 2 engineers forget about it
    if (state.numCardsOfType(_playerID, engType) != 2)
    {
        return;   
    }
    
    // if the enemy has tech already then exit
    const PlayerID enemy = state.getEnemy(_playerID);
    const std::vector<std::string> enemyCantHave = { "Animus", "Blastforge", "Conduit", "Chronofilter" };
    std::vector<CardType> enemyCantHaveType;
    for (const auto & name : enemyCantHave)
    {
        if (CardTypes::CardTypeExists(name))
        {
            enemyCantHaveType.push_back(CardTypes::GetCardType(name));
        }
    }

    // check for those types
    for (auto & type : enemyCantHaveType)
    {
        if (state.numCardsOfType(enemy, type) > 0)
        {
            return;
        }
    }

    // do the engineer buying heuristic
    size_t fourHealthBlockers = 0;
    size_t bigBlueCards = 0;
    for (size_t c(0); c < state.numCardsBuyable(); ++c)
    {
        const CardType & type = state.getCardBuyableByIndex(c).getType();

        if (type.canBlock(false) && (!type.isFragile()) && (type.getStartingHealth() >= 4) && (type.getAttack() <= (type.getStartingHealth()/2)))
        {
            fourHealthBlockers++;
        }

        if (type.getBuyCost().amountOf(Resources::Blue) >= 2)
        {
            bigBlueCards++;
        }
    }

    if (fourHealthBlockers > 0 || bigBlueCards > 2)
    {
        state.doAction(buyEngineer);
        move.addAction(buyEngineer);
    }
}


/* Necessary conditions for 3rd engineer:

- Opponent hasn't bought ABC or Chrono Filter yet
- Thorium Dynamo isn't available (if it is, 3rd Engineer is not necessary)
- If bot is p2, Trinity Drone is not available (if it is, DD/DDC is fine and 3rd Engineer is not necessary)
- You score at least 9 points in the following list if you're p1, and 10 points if you're p2:
    Doomed Drone: worth 6 points
    Vivid Drone: worth 50 points
    Aegis: worth 4 points
    Doomed Wall: worth 14 points
    Infusion Grid: worth 12 points
    Xeno Guardian: worth 11 points
    Doomed Mech: worth 9 points
    Energy Matrix: worth 20 points
    Omega Splitter: worth 7 points
    Arka Sodara: worth 6 points
    Centurion: worth 20 points
    Polywall: worth 5 points
    Plexo: worth 5 points
    Frostbite: worth -2 points
    Electrovore: worth -3 points
    Grimbotch: worth -1 points
    Shadowfang: worth -4 points
    Cynestra: worth -1 points
    Tia: worth -5 points
    Cryo Ray: worth -2 points

*/