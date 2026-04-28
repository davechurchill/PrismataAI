#include "PartialPlayer_ActionBuy_Random.h"
#include "Random.h"

using namespace Prismata;

PartialPlayer_ActionBuy_Random::PartialPlayer_ActionBuy_Random(const PlayerID playerID)
{
 _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

void PartialPlayer_ActionBuy_Random::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    std::vector<Action> buyActions;
    while (state.getActivePlayer() == _playerID && state.getActivePhase() == Phases::Action)
    {
        if (move.size() >= MAX_MOVE_ACTIONS)
        {
            return;
        }

        buyActions.clear();
        for (CardID c(0); c < state.numCardsBuyable(); ++c)
        {
            const CardType cardBuyableType = state.getCardBuyableByIndex(c).getType();
            const Action buyCardAction(_playerID, ActionTypes::BUY, cardBuyableType.getID());

            if (state.isLegal(buyCardAction))
            {
                buyActions.push_back(buyCardAction);
            }
        }

        if (buyActions.empty())
        {
            return;
        }

        Action a = buyActions[Random::Int(buyActions.size())];
        state.doAction(a);
        move.addAction(a);
    }
}
