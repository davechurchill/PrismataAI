#include "PartialPlayer_Breach_GreedyKnapsack.h"

using namespace Prismata;

PartialPlayer_Breach_GreedyKnapsack::PartialPlayer_Breach_GreedyKnapsack(const PlayerID playerID, bool lowTechPriority, EvaluationType (*heuristic)(const Card &, const GameState &, const HealthType))
    : _heuristic(heuristic)
    , _totalBreachDamageLoss(0)
    , _lowTechPriority(lowTechPriority)
{
    _playerID = playerID;
    _phaseID = PPPhases::BREACH;
}

void PartialPlayer_Breach_GreedyKnapsack::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Breach)
    {
        return;
    }

    _totalBreachDamageLoss = 0;
    
    const PlayerID enemy = state.getEnemy(_playerID);

    // end phase action
    const Action end(state.getActivePlayer(), ActionTypes::END_PHASE, 0);

    // keep breaching while we are in the breach phase and we can't end it by passing
    bool didUndoChill = false;
    while ((state.getActivePhase() == Phases::Breach) && !state.isLegal(end))
    {
        bool foundBreachTarget          = false;
        bool foundFrozenTarget          = false;
        bool foundFrozenFragileLarger   = false;
        CardID frozenFragileLargerID    = 0;
        CardID bestBreachTargetID       = 0;
        double bestBreachTargetLoss     = std::numeric_limits<double>::lowest();
        double bestBreachTrueTargetLoss = std::numeric_limits<double>::lowest();

        //std::cout << "New Breach Iteration\n";

        // find the enemy card to breach next
        //std::cout << "Looking at " << state.numCards(enemy) << " enemy cards..\n";
        for (const auto & cardID : state.getCardIDs(enemy))
        {
            const Card & card = state.getCardByID(cardID);
            const Action breachCard(_playerID, ActionTypes::ASSIGN_BREACH, card.getID());

            // figure out the value of actually breaching this card for this much damage
            HealthType breachDamageToAssign = std::min(card.currentHealth(), state.getAttack(_playerID));
            double breachTrueTargetLoss = _heuristic(card, state, breachDamageToAssign);
            double breachDamageLoss = breachTrueTargetLoss / card.currentHealth();

            // lower the breach damage loss of tech types if we're using low tech priority breaching
            if (_lowTechPriority && card.getType().isTech())
            {
                breachDamageLoss /= 100000.0;
            }

            //std::cout << "   Card: " << card.getType().getUIName() << " " << breachDamageToAssign << " " << breachTrueTargetLoss << " " << breachDamageLoss << "\n";

            // if we found a new best target
            if (breachDamageLoss > bestBreachTargetLoss)
            {
                // if it's a legal breach action, set the new best
                if (state.isLegal(breachCard))
                {
                    foundBreachTarget = true;
                    foundFrozenTarget = false;
                    bestBreachTargetID = card.getID();
                    bestBreachTargetLoss = breachDamageLoss;
                    bestBreachTrueTargetLoss = breachTrueTargetLoss;

                    //std::cout << "   Found Breachable " << card.getType().getUIName() << " " << breachDamageLoss << "\n";
                }
                // otherwise it's a frozen target
                else if (card.isFrozen())
                {
                    // this checks if we can't actually breach this target
                    if (state.getAttack(_playerID) < card.currentHealth())
                    {
                        if (card.getType().isFragile())
                        {       
                            foundFrozenFragileLarger = true;
                            frozenFragileLargerID = card.getID();
                        }

                        continue;
                    }

                    foundFrozenTarget = true;
                    foundBreachTarget = false;
                    bestBreachTargetID = card.getID();
                    bestBreachTargetLoss = breachDamageLoss;
                    bestBreachTrueTargetLoss = breachTrueTargetLoss;

                    //std::cout << "   Found Frozen Br " << card.getType().getUIName() << " " << breachDamageLoss << "\n";
                }
                else
                {
                    //std::cout << "   Not breachable: " << card.getID() << "\n";
                }
            }
        }

        PRISMATA_ASSERT(foundBreachTarget || foundFrozenTarget || foundFrozenFragileLarger, "Should have found a breach target or frozen card from %d cards.\n\n %s", state.numCards(enemy), move.toString().c_str());

        // if we found a legal breach target, we can simply breach it and continue
        if (foundBreachTarget)
        {
            const Action breachCard(_playerID, ActionTypes::ASSIGN_BREACH, bestBreachTargetID);

            state.doAction(breachCard);
            move.addAction(breachCard);

            _totalBreachDamageLoss += bestBreachTrueTargetLoss;
        }
        // otherwise we found a breachable frozen card or a fragile larger card
        else if (foundFrozenTarget || foundFrozenFragileLarger)
        {
            Action undoChill(_playerID, ActionTypes::UNDO_CHILL, foundFrozenTarget ? bestBreachTargetID : frozenFragileLargerID);

            PRISMATA_ASSERT(state.isLegal(undoChill), "Undoing chill should always be legal");

            // undo the chill, this will revert us to action phase
            state.doAction(undoChill);
            move.addAction(undoChill);

            PRISMATA_ASSERT(state.getActivePhase() == Phases::Action, "We should be in action phase after a chill undo during breach");

            // this will effectively breach the target by doing a wipeout, so don't issue the breach action
            state.doAction(end);
            move.addAction(end);
            
            // record if we undid a chill, in case we no longer enter a breach phase
            didUndoChill = true;

            if (!foundFrozenTarget)
            {
                PRISMATA_ASSERT(state.isLegal(end), "No breachable found, and space is not legal");
            }
            else
            {
                _totalBreachDamageLoss += bestBreachTrueTargetLoss;
            }
        }
    }

    // we now have to be in one of two states: at the end of breach, or we unded the freeze on a card we can't kill so we went directly to confirm
    //PRISMATA_ASSERT(state.getActivePhase() == Phases::Breach || didUndoChill, tempState, "test.png",  "We should be in breach phase: %s", move.toString().c_str());

    // if we are still in the breach phase, take us to confirm
    if (state.getActivePhase() == Phases::Breach)
    {
        PRISMATA_ASSERT(state.isLegal(end), "Should be able to end breach phase");

        state.doAction(end);
        move.addAction(end);
    }   
}

double PartialPlayer_Breach_GreedyKnapsack::getTotalBreachDamageLoss() const
{
    return _totalBreachDamageLoss;
}
