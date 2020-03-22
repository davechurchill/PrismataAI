#include "PartialPlayer_ActionBuy_Combination.h"

using namespace Prismata;

PartialPlayer_ActionBuy_Combination::PartialPlayer_ActionBuy_Combination(const PlayerID playerID, const std::vector<PPPtr> & combo)
{
    _movesGenerated = std::vector<size_t>(combo.size(), 0);

    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
    _combo = combo;
}

void PartialPlayer_ActionBuy_Combination::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    std::fill(_movesGenerated.begin(), _movesGenerated.end(), 0);

    // get the moves from each player in the combo
    for (size_t p(0); p < _combo.size(); ++p)
    {
        size_t movesBefore = move.size();
        _combo[p]->getMove(state, move);
        _movesGenerated[p] = move.size() - movesBefore;
    }
}

PPPtr PartialPlayer_ActionBuy_Combination::clone() 
{ 
    std::vector<PPPtr> newCombo;

    for (size_t i(0); i<_combo.size(); ++i)
    {
        newCombo.push_back(_combo[i]->clone());
    }

    PPPtr combo(new PartialPlayer_ActionBuy_Combination(_playerID, newCombo));
    combo->setDescription(_description);
    return combo;
}

void PartialPlayer_ActionBuy_Combination::setBuyLimits(const BuyLimits & buyLimits)
{
    _buyLimits = buyLimits;

    for (size_t i(0); i<_combo.size(); ++i)
    {
        BuyLimits limits = _combo[i]->getBuyLimits();
        limits.addLimits(buyLimits);
        _combo[i]->setBuyLimits(limits);
    }
}

std::string PartialPlayer_ActionBuy_Combination::getDescription(size_t depth) 
{ 
    std::stringstream ss; 

    for (size_t i(0); i < depth; ++i)
    {
        ss << " ";
    }

    ss << _description << "\n";
    
    for (size_t i(0); i<_combo.size(); ++i) 
    { 
        ss << _movesGenerated[i] << " ";
       
        ss << _combo[i]->getDescription(depth + 1);

        if (i < _combo.size() - 1)
        {
            ss << "\n";
        }
    } 
    
    return ss.str(); 
}