#include "PartialPlayer_ActionBuy_Sequence.h"

using namespace Prismata;

PartialPlayer_ActionBuy_Sequence::PartialPlayer_ActionBuy_Sequence(const PlayerID & playerID, const BuySequence & buySequence)
    : _buySequence(buySequence)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

void PartialPlayer_ActionBuy_Sequence::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    if (state.getResources(0).amountOf(Resources::Gold) == 8)
    {
        int a = 6;
    }

    for (size_t i(0); i<_buySequence.size(); ++i)
    {
        const CardType & type = _buySequence[i].first;

        Action buy(_playerID, ActionTypes::BUY, type.getID());
        size_t bought(0);

        while (true)
        {
            if ((_buySequence[i].second > 0) && (bought >= _buySequence[i].second))
            {
                break;
            }

            const CardID numOwned = state.numCardsOfType(_playerID, type);
            if (_buyLimits.hasLimit(type) && (numOwned >= _buyLimits.getLimit(type)))
            {
                break;
            }

            if (!state.isLegal(buy))
            {
                break;
            }

            state.doAction(buy);
            move.addAction(buy);
            ++bought;
        }
    }

}
