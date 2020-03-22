#include "PartialPlayer_ActionAbility_SnipeGreedyKnapsack.h"

using namespace Prismata;

PartialPlayer_ActionAbility_SnipeGreedyKnapsack::PartialPlayer_ActionAbility_SnipeGreedyKnapsack(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState &))
    : _heuristic(heuristic)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_SnipeGreedyKnapsack::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
    
    // make a vector of all our snipers
    std::map< CardType, std::vector<CardID> > ourSniperMap;
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);

        if (card.getType().hasTargetAbility() && card.getType().getTargetAbilityType() == ActionTypes::SNIPE && card.canUseAbility())
        {
            if (ourSniperMap.find(card.getType()) == ourSniperMap.end())
            {
                ourSniperMap[card.getType()] = std::vector<CardID>();
            }

            ourSniperMap[card.getType()].push_back(cardID);
        }
    }

    // if we got nothing to do don't do nothing
    if (ourSniperMap.empty())
    {
        return;
    }

    // iterate through each sniper type that we have
    for (auto & kv : ourSniperMap)
    {
        // get the first sniper in the vector for legality checks
        const CardID sniperCardID = kv.second.front();
        Action sniperClick(_playerID, ActionTypes::USE_ABILITY, sniperCardID);
        Action undoSniperClick(_playerID, ActionTypes::UNDO_USE_ABILITY, sniperCardID);

        if (!state.isLegal(sniperClick))
        {
            continue;
        }

        state.doAction(sniperClick);

        //std::cout << "Sniper: " << state.getCardByID(sniperCardID).getType().getUIName() << std::endl; 
        
        // create a vector of legal targets for this sniper type
        std::vector<CardID> enemySnipableCardIDs;
        for (const auto & targetID : state.getCardIDs(state.getEnemy(_playerID)))
        {
            Action snipe(_playerID, ActionTypes::SNIPE, sniperCardID, targetID);    

            if (state.isLegal(snipe))
            {
                //std::cout << "    Target: " << state.getCardByID(targetID).getType().getUIName() << std::endl; 
                enemySnipableCardIDs.push_back(targetID);
            }
        }

        state.doAction(undoSniperClick);

        // if they have no snipable cards then go to the next type
        if (enemySnipableCardIDs.empty())
        {
            continue;
        }

        // sort the cards in the given snipe priority
        std::sort(enemySnipableCardIDs.begin(), enemySnipableCardIDs.end(), SnipeKnapsackCompare(_heuristic, state));

        // iterate through our snipers of this type, sniping in the given priority
        for (const auto & sniperID : kv.second)
        {
            Action sniperClick(_playerID, ActionTypes::USE_ABILITY, sniperID);

            if (!state.isLegal(sniperClick))
            {
                continue;
            }

            // we have to first click the sniper for the snipe action to be legal
            state.doAction(sniperClick);

            bool snipeFound = false;
            for (size_t t(0); t<enemySnipableCardIDs.size(); ++t)
            {
                Action snipe(_playerID, ActionTypes::SNIPE, sniperID, enemySnipableCardIDs[t]);

                if (state.isLegal(snipe))
                {
                    //std::cout << "        Killing: " << state.getCardByID(enemySnipableCardIDs[t]).getType().getName() << std::endl;
                    snipeFound = true;
                    move.addAction(sniperClick);
                    state.doAction(snipe);
                    move.addAction(snipe);
                    break;
                }
            }
            
            // if we didn't find a snipe target we have to undo the snipe click
            if (!snipeFound)
            {
                Action undoSniperClick(_playerID, ActionTypes::UNDO_USE_ABILITY, sniperID);

                PRISMATA_ASSERT(state.isLegal(undoSniperClick), "undo sniperClick should be legal");

                state.doAction(undoSniperClick);
            }
        }
    }
}
