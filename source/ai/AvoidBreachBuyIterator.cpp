#include "AvoidBreachBuyIterator.h"
#include "AITools.h"
#include "ChillIterator.h"

using namespace Prismata;

/* 
 * AvoidBreachBuyIterator class attempts to calculate how to buy prompt blockers in order to avoid being breached
 * Based on input parameters it will also attempt to untap drones if it is advantageous to do so
 *
 * The main heuristic used in this calculation is the sum total resource cost that will be lost if a given
 * purchasing sequence is used. For example, if a wall is purchased and can stop all of the incoming damage
 * then the loss will be zero, but if the wall dies then the cost will be the cost of a wall. If a wall and
 * and engineer will die then the cost is 7BE which is Wall + Engineer. Opponent breach damage is also
 * considered by this algorithm, so for example if buying a wall will stop 3 tarsiers from being breached
 * then the algorithm will figure this out and do it. This sum of resource cost loss is not a perfect heuristic,
 * but in practice it has worked fairly well so far.
 *
 * The logic flow for this class is as follows:
 *
 *   Constructor - Preprocess the state, calculating predicted incoming damage from enemy, etc
 *
 *   solve()     - Function which is called externally to do the minimzation solve
 *               - First checks to see if any single wall can be bought to absorb damage
 *               - Next if any single blocker can absorb, buy the cheapest one that can do so
 *               - If no single blocker can absorb, call the recurse function to iterate over all purchase sequences
 *
 *   recurse()   - Recursive function which calculates the min cost prompt blocking purchase sequence
 *               - Brute force checks all prompt purchase sequences, keeping track of the min cost solution
 *               - For each sequence, checks to see if untapping drones will help minimize the cost
 *               - Stores the best sequence found in _bestActionSequence vector
 *               - Stores the number of drones to untap in _bestUntapDrones
 *
 *   getMove()  - Computes the individual actions to return based on the solution found above
 *               - Will untap doomed drones before drones, etc
 */
AvoidBreachBuyIterator::AvoidBreachBuyIterator(const GameState & state, const BreachIteratorParameters & params)
    : _print                    (false)
    , _nodesSearched            (0)
    , _state                    (state)
    , _initialState             (state)
    , _predictedNextTurnState   (state)
    , _playerID                 (state.getActivePlayer())
    , _predictedEnemyAttack     (0)
    , _enemyChillPotential      (0)
    , _ourLargestBlocker        (0)
    , _enemyMaxBreachPotential  (0)
    , _ourAvailableDefense      (0)
    , _minSolutionCost          (std::numeric_limits<double>::max(), std::numeric_limits<double>::max())
    , _minTrueCost              (std::numeric_limits<double>::max())
    , _solutionFound            (false)
    , _bestUntapDrones          (0)
    , _numTappedDrones          (0)
    , _originalBlockLoss        (0)
    , _params                   (params)
{
    _actionStack.reserve(20);

    // First we have to set up the cards that we care about for this class
    // Add the prompt defense cards to the list of buyables
    // Only these cards will be considered in the recursion
    for (size_t c(0); c < state.numCardsBuyable(); ++c)
    {
        const CardType type = state.getCardBuyableByIndex(c).getType();

        if (type.isPromptBlocker())
        {
            _promptCardBuyableIndex.push_back(c);
        }
        else
        {
            // add any prompt blocking cards created by this card's buy script
            bool createsPromptBlocker = false;            
            const std::vector<CreateDescription> & created = type.getBuyScript().getEffect().getCreate();
            for (size_t j(0); j < created.size(); ++j)
            {
                if (created[j].getOwn())
                {
                    if (created[j].getType().isPromptBlocker())
                    {
                        createsPromptBlocker = true;
                        break;
                    }
                }
            }
            
            if (createsPromptBlocker)
            {
                _promptCardBuyableIndex.push_back(c);
            }
        }
    }

    // We sort the prompt blockers so that we compare higher health blockers first
    // This heuristic usually helps cut down nodes searched by considering better solutions first
    std::sort(std::begin(_promptCardBuyableIndex), std::end(_promptCardBuyableIndex), PromptBlockerCompare(_state));

    // Estimate how much attack our opponent can send at us next turn
    // This will be used as the incoming damage amount that we're going to be defending against
    AITools::PredictEnemyNextTurn(_predictedNextTurnState);
    _predictedEnemyAttack = _predictedNextTurnState.getAttack(_state.getInactivePlayer());

    // The AttackDamageScenario class is used for calculating how much damage a defender will incur
    // given their current blockers and an incoming attack amount. This class memoizes computed values
    // which is necessary since our recursion will be doing a lot of damage checks
    _attackDamageScenario = AttackDamageScenario(_predictedNextTurnState);
    
    if (_print) { std::cout << "\nPredicted Enemy Attack: " << _predictedEnemyAttack << "\n"; }
    
    // Calculate the maximum possible chill that the enemy can produce
    PlayerID enemy = _predictedNextTurnState.getEnemy(_playerID);
    for (const auto & cardID : _predictedNextTurnState.getCardIDs(enemy))
    {
        const Card & card = _predictedNextTurnState.getCardByID(cardID);

        bool hasTargetAbility = card.getType().hasTargetAbility();
        bool hasChill = card.getType().getTargetAbilityType() == ActionTypes::CHILL;
        bool constr = card.isUnderConstruction();
        bool delayed = card.isDelayed();

        if (card.getType().hasTargetAbility() && card.getType().getTargetAbilityType() == ActionTypes::CHILL && !card.isUnderConstruction() && !card.isDelayed())
        {
            _enemyChillPotential += card.getType().getTargetAbilityAmount();
        }
    }

    _chillScenario = ChillScenario(_predictedNextTurnState, enemy);

    if (_print) { std::cout << "Predicted Enemy Chill:  " << _enemyChillPotential << "\n"; }
        
    // calculate our predicted available defense
    _initialFrontlineUsed = 0;
    for (const auto & cardID : _predictedNextTurnState.getCardIDs(_playerID))
    {
        const Card & card = _predictedNextTurnState.getCardByID(cardID);

        if (card.canBlock())
        {
            _ourAvailableDefense += card.currentHealth();
        }

        bool isAbsorber = !card.getType().isFragile();
        HealthType attackRemaining = _predictedEnemyAttack - _initialFrontlineUsed;
        if (card.getType().isFrontline() && attackRemaining >= card.currentHealth())
        {
            isAbsorber = false;
            _initialFrontlineUsed += card.currentHealth();
        }

        if (card.canBlock() && isAbsorber)
        {
            if (card.currentHealth() > _ourLargestBlocker)
            {
                _ourLargestBlocker = card.currentHealth();
                _ourLargestBlockerCost = BuyCost(HeuristicValues::Instance().GetBuyManaCost(card.getType()), HeuristicValues::Instance().GetBuySacCost(card.getType()));
            }
        }
    }

    int ourDefenseAfterChill    = std::max(0, _ourAvailableDefense - _enemyChillPotential);
    _enemyMaxBreachPotential = (_predictedEnemyAttack > ourDefenseAfterChill) ? (_predictedEnemyAttack - ourDefenseAfterChill) : 0;

    if (_print) { std::cout << "Predicted Enemy Max Breach: " << _enemyMaxBreachPotential << "\n"; }

    if (CardTypes::CardTypeExists("Drone"))
    {
        _droneExists = true;
        _droneType = CardTypes::GetCardType("Drone");
    }

    if (CardTypes::CardTypeExists("Doomed Drone"))
    {
        _doomedDroneExists = true;
        _doomedDroneType = CardTypes::GetCardType("Doomed Drone");
    }

    if (CardTypes::CardTypeExists("Wall"))
    {
        _wallExists = true;
        _wallType = CardTypes::GetCardType("Wall");
    }

    // calcuate how many tapped drones we have
    if (_droneExists)
    {
        for (const auto & cardID : _state.getCardIDs(_playerID))
        {
            const Card & card = _state.getCardByID(cardID);

            if (card.getType() != _droneType && card.getType() != _doomedDroneType)
            {
                continue;
            }

            const Action untapDrone(_playerID, ActionTypes::UNDO_USE_ABILITY, card.getID());

            if (state.isLegal(untapDrone))
            {
                _numTappedDrones++;
            }
        }
    }
}

void AvoidBreachBuyIterator::getMove(Move & move)
{
    GameState untapState(_initialState);
    
    for (size_t i(0); i < _bestActionSequence.size(); ++i)
    {
        PRISMATA_ASSERT(untapState.isLegal(_bestActionSequence[i]), "This best action should be legal");

        move.addAction(_bestActionSequence[i]);
        untapState.doAction(_bestActionSequence[i]);
    }

    // figure out which drones we can untap then sort them by priority
    std::vector<CardID> untappableDroneIDs;
    for (const auto & cardID : untapState.getCardIDs(_playerID))
    {
        const Card & card = untapState.getCardByID(cardID);
        const Action untapDrone(_playerID, ActionTypes::UNDO_USE_ABILITY, card.getID());
        
        // add drones and doomed drones to the list
        if (((card.getType() == _droneType) || (card.getType() == _doomedDroneType)) && (untapState.isLegal(untapDrone)))
        {
            untappableDroneIDs.push_back(card.getID());
        }
    }

    std::sort(std::begin(untappableDroneIDs), std::end(untappableDroneIDs), DroneBlockCompare(untapState));

    // do the drone untapping
    CardID untappedDrones = 0;
    for (; untappedDrones < _bestUntapDrones; ++untappedDrones)
    {
        const Card & card = untapState.getCardByID(untappableDroneIDs[untappedDrones]);
        const Action untapDrone(_playerID, ActionTypes::UNDO_USE_ABILITY, card.getID());

        untapState.doAction(untapDrone);
        move.addAction(untapDrone);
    }

    PRISMATA_ASSERT(untappedDrones == _bestUntapDrones, "Didn't find enough drones to untap: %d %d %d", untappedDrones, _bestUntapDrones, _numTappedDrones);
}

void AvoidBreachBuyIterator::setBestSequence(CardID untapDrones)
{
    _bestUntapDrones = untapDrones;
    _bestActionSequence.clear();

    for (size_t i(0); i < _actionStack.size(); ++i)
    {
        _bestActionSequence.push_back(_actionStack[i]);   
    }
}

void AvoidBreachBuyIterator::solve()
{
    BuyCost largestAbsorberCost;

    if (_ourLargestBlocker > 1)
    {
        largestAbsorberCost = _ourLargestBlockerCost;
    }

    // if we can already avoid damage we don't need to solve anything
    if (_predictedEnemyAttack == 0 || (_ourLargestBlocker > _enemyChillPotential && _ourLargestBlocker > _predictedEnemyAttack))
    {
        return;
    }

    // if we can buy a single wall which will let us avoid breach, do it
    if (_wallExists && 
        (_predictedEnemyAttack < _wallType.getStartingHealth()) && // wall will absorb everything
        (_enemyChillPotential < _wallType.getStartingHealth()) &&  // wall can't be frozen
        (_ourLargestBlocker < _wallType.getStartingHealth()))      // we don't already have a wall
    {
        const Action buyWall(_state.getActivePlayer(), ActionTypes::BUY, _wallType.getID());

        if (_state.isLegal(buyWall))
        {
            _actionStack.push_back(buyWall);
            setBestSequence(0);
            return;
        }
    }

    // check to see if we can buy a single blocker which can absorb the entire enemy attack without being frozen
    bool absorberFound = false;
    CardType cheapestAbsorber;
    //for (CardID c(0); c < _promptAbsorberCardBuyableIndex.size(); ++c)
    for (CardID c(0); c < _promptCardBuyableIndex.size(); ++c)
    {
        const CardType cardType = _state.getCardBuyableByIndex(_promptCardBuyableIndex[c]).getType();

        // filter out non-absorbers
        if (cardType.isFragile() || (cardType.getLifespan() == 1) || (cardType.getStartingHealth() == 1))
        {
            continue;
        }

        const Action buyCard(_state.getActivePlayer(), ActionTypes::BUY, cardType.getID());    
    
        // candidate absorber test
        if ((cardType.getStartingHealth() > _predictedEnemyAttack) && (cardType.getStartingHealth() > _enemyChillPotential) && _state.isLegal(buyCard))
        {
            if (!absorberFound || (HeuristicValues::Instance().GetBuyTotalCost(cardType) < HeuristicValues::Instance().GetBuyTotalCost(cheapestAbsorber)))
            {
                absorberFound = true;
                cheapestAbsorber = cardType;
            }
        }
    }

    // if we found a single absorber to solve the problem, do it
    if (absorberFound)
    {
        const Action buyAbsorber(_state.getActivePlayer(), ActionTypes::BUY, cheapestAbsorber.getID());    

        _state.doAction(buyAbsorber);
        _actionStack.push_back(buyAbsorber);
        setBestSequence(0);
        return;
    }

    // If the problem is more complex than this, use the solver
    recurse(0, 0, _ourLargestBlocker, largestAbsorberCost, BuyCost(), 0, _initialFrontlineUsed);
}

void AvoidBreachBuyIterator::recurse(const CardID currentCardBuyableIndex, const size_t numBought, const HealthType largestAbsorberHealth, const BuyCost largestAbsorberCost, const BuyCost cost, const HealthType defenseBought, const HealthType frontlineAssigned)
{
    // Only check to see if we have a new best solution if something changed since the last recursion
    //   if currentCardBuyAbleIndex == 0 this is the root, so check the root solution
    //   if numBought > 0 then something new has been bought so we need to check the solution
    //   in all other cases nothing has changed so we know we won't have a new best solution
    if ((currentCardBuyableIndex == 0) || (numBought > 0))
    {
        // For the current buy sequence we are exploring, check to see if untapping drones will help 
        CardID maxUntappableDrones = calculateNumUntappableDrones(_state);
        for (CardID untapDrones(0); untapDrones <= maxUntappableDrones; ++untapDrones)
        {
            ++_nodesSearched;

            // If we are untapping a drone, modify the chill scenario to reflect the extra drone on defense
            if (untapDrones > 0)
            {
                _chillScenario.addDefender(_droneType);
            }

            // Calculate how much chill the enemy is able to use against us and how much defense we will have left
            HealthType enemyChill        = calculateEnemyChillUsage();
            HealthType totalDefense      = (HealthType)(_ourAvailableDefense + defenseBought + untapDrones);
            HealthType defenseAfterChill = (enemyChill >= totalDefense) ? 0 : (totalDefense - enemyChill);
            HealthType enemyBreachAmount = (defenseAfterChill >= _predictedEnemyAttack) ? 0 : (_predictedEnemyAttack - defenseAfterChill);

            // Add the cost of untapping a the number of drones we have chosen to untap
            BuyCost untapDroneCost       = cost + BuyCost(0, untapDrones * (HeuristicValues::Instance().GetBuyTotalCost(_droneType) + 1));

            // The 'true cost' is the cost of purchasing plus the damage we will incur if we are breached
            // an enemy breach of 0 means that we will incur the damage of blocking with our initially held blockers
            double trueCost              = untapDroneCost.resourceCost + untapDroneCost.lossCost + _attackDamageScenario.getBreachLoss(enemyBreachAmount);
            double spent                 = trueCost;

            // If we can absorb something, remove our largest cost absorber from the true cost
            if ((_predictedEnemyAttack < defenseAfterChill) && (largestAbsorberHealth > 1))
            {
                trueCost = trueCost - largestAbsorberCost.resourceCost;
            }

            // Elyot's chill fudging heuristic
            trueCost = std::max(0.0, trueCost - 2.0 * enemyChill);

            // Print the current solution (debugging)
            printStack(_state, largestAbsorberHealth, largestAbsorberCost, untapDroneCost, _ourAvailableDefense + defenseBought, untapDrones, spent, trueCost, enemyChill);

            // Update our current best solution if we have found a new best true cost
            if ((trueCost < _minTrueCost) || ((trueCost == _minTrueCost) && (spent < _minSpent)))
            {
                _minTrueCost = trueCost;
                _minSpent = spent;
                setBestSequence(untapDrones);
                //printStack(_state, largestAbsorberHealth, largestAbsorberCost, untapDroneCost, _ourAvailableDefense + defenseBought, untapDrones, trueCost);
            }
        }

        // Remove the drones that we added to the defense scenario above
        _chillScenario.removeDefender(_droneType, maxUntappableDrones);
    }

    // if we have run out of new things to try to buy, we can prune this branch
    if (currentCardBuyableIndex >= _promptCardBuyableIndex.size())
    {
        return;
    }

    // set up the next card type to buy
    const CardType cardType = _state.getCardBuyableByIndex(_promptCardBuyableIndex[currentCardBuyableIndex]).getType();
    const Action buyCard(_state.getActivePlayer(), ActionTypes::BUY, cardType.getID());

    // go down the branch we we buy one of the current card type
    if (_state.isLegal(buyCard))
    {
        _state.doAction(buyCard);
        _actionStack.push_back(buyCard);
        _chillScenario.buyDefender(cardType);
        _predictedEnemyAttack += cardType.getAttackGivenToEnemy();

        CardID cardBoughtID                 = _state.getLastCardBoughtID();
        BuyCost cardCost                    = BuyCost(HeuristicValues::Instance().GetBuyManaCost(cardType) + HeuristicValues::Instance().GetInflatedManaCostValueGivenToEnemy(cardType), HeuristicValues::Instance().GetBuySacCost(cardType));
        BuyCost newCost                     = cost + cardCost;
        HealthType newLargestAbsorberHealth = largestAbsorberHealth;
        BuyCost newLargestAbsorberCost      = largestAbsorberCost;

        bool isAbsorber = !cardType.isFragile();
        HealthType frontlineUsed = 0;

        HealthType attackRemaining = _predictedEnemyAttack - frontlineAssigned;
        if (cardType.isFrontline() && attackRemaining >= cardType.getStartingHealth())
        {
            isAbsorber = false;
            frontlineUsed = cardType.getStartingHealth();
        }

        if (isAbsorber)
        {
            if (cardType.getStartingHealth() > largestAbsorberHealth)
            {
                newLargestAbsorberHealth = cardType.getStartingHealth();
                newLargestAbsorberCost = cardCost;
            }
            else if (cardType.getStartingHealth() == largestAbsorberHealth)
            {
                if (largestAbsorberCost < cardCost)
                {
                    newLargestAbsorberHealth = cardType.getStartingHealth();
                    newLargestAbsorberCost = cardCost;
                }
            }
        }

        // determine if any drones or doomed drones were sacced by buying this card
        const CardID droneDifference = cardType.getTypeBuySacCost(_droneType) + cardType.getTypeBuySacCost(_doomedDroneType);
        _numTappedDrones -= droneDifference;
        
        recurse(currentCardBuyableIndex, 
                numBought + 1, 
                newLargestAbsorberHealth,
                newLargestAbsorberCost,
                newCost, 
                _chillScenario.getTotalDefenseBought(),
                frontlineAssigned + frontlineUsed);
        
        _actionStack.pop_back();
        _chillScenario.sellDefender(cardType);
        _predictedEnemyAttack -= cardType.getAttackGivenToEnemy();

        // put the sacced drones back
        _numTappedDrones += droneDifference;

        const Action sell(buyCard.getPlayer(), ActionTypes::SELL, cardBoughtID);
        _state.doAction(sell);
    }

    // go down the branch where we skip buying more of the current card type and go to the next type
    recurse(currentCardBuyableIndex + 1, 0, largestAbsorberHealth, largestAbsorberCost, cost, defenseBought, frontlineAssigned);
}

HealthType AvoidBreachBuyIterator::calculateEnemyChillUsage()
{
    // The default estimation is that the enemy can use all available chill against us
    HealthType enemyChill = _enemyChillPotential;

    // The SOLVER method actually iterates through all chill sequences and determines how much it can actually apply
    if (_params.chillCalculationMethod == ChillCalculationMethod::SOLVER)
    {
        enemyChill = _chillScenario.calculateUsedChill(_params.maxChillSolverIterations);
    }
    // The HEURISTIC guess does a decent job of guessing how much chill the enemy can use
    else if (_params.chillCalculationMethod == ChillCalculationMethod::HEURISTIC)
    {
        enemyChill = _chillScenario.calculateHeuristicUsedChill();
    }

    return enemyChill;
}

CardID AvoidBreachBuyIterator::calculateNumUntappableDrones(const GameState & state)
{    
    if (!_droneExists)
    {
        return 0;
    }

    CardID maxDronesToUntap = _params.maxUntapDrones;
    maxDronesToUntap        = std::min(maxDronesToUntap, _numTappedDrones);
    maxDronesToUntap        = std::min(maxDronesToUntap, (CardID)state.getResources(_playerID).amountOf(Resources::Gold));
    maxDronesToUntap        = std::min(maxDronesToUntap, (CardID)_predictedEnemyAttack);

    return maxDronesToUntap;
}

size_t AvoidBreachBuyIterator::getNodesSearched() 
{
    return _nodesSearched;
}

void AvoidBreachBuyIterator::printStack(const GameState & state, HealthType largestAbsorberHealth, BuyCost largestAbsorberCost, BuyCost cost, HealthType defenseBought, CardID untapDrones, double spent, double trueCost, HealthType chill)
{
    if (!_print)
    {
        return;
    }

    int ourDefenseAfterChill    = std::max(0, (int)((int)defenseBought + untapDrones - chill));
    int enemyBreachAmount       = (int)_predictedEnemyAttack - ourDefenseAfterChill;
    HealthType breach           = enemyBreachAmount > 0 ? enemyBreachAmount : 0;
    double absorbSavings        = 0;

    std::stringstream breachss;
    if (enemyBreachAmount >= 0)
    {
        breachss << enemyBreachAmount;
    }
    else
    {
        breachss << "-";
    }

    // if we can absorb something, remove our largest cost absorber from the true cost
    if (enemyBreachAmount < 0 && largestAbsorberHealth > 1)
    {
        absorbSavings = largestAbsorberCost.resourceCost;
    }

    printf("%7d [%9.2lf,%9.2lf] %8d %8d %8d %8s %9.2lf %9.2lf %9.2lf %9.2lf   | ", untapDrones, cost.resourceCost, cost.lossCost, _predictedEnemyAttack, chill, ourDefenseAfterChill, breachss.str().c_str(), absorbSavings, _attackDamageScenario.getBreachLoss(breach), spent, trueCost);
        
    std::cout << untapDrones << "D, ";

    for (size_t i(0); i<_actionStack.size(); ++i)
    {
        std::cout << ((i > 0) ? ", " : " ") << CardTypes::GetAllCardTypes()[_actionStack[i].getID()].getUIName();
    }   std::cout << std::endl;
}


void AvoidBreachBuyIterator::debugSolve()
{
    _print = true;
    if (_print)
    {
        printf("\nUnDrone   ManaCost   LossCost   Attack    Chill   OurDef   Breach  Absorbed   DefLoss     Spent  RealCost   | Solution\n");
        printf("---------------------------------------------------------------------------------------------------------------------------------------\n");
    }

    solve();
    _print = false;
}


void AvoidBreachBuyIterator::setPrint(bool print)
{
    _print = print;
}
