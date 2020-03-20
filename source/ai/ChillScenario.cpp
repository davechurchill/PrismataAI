#include "ChillScenario.h"
#include "Timer.h"

using namespace Prismata;

ChillScenario::ChillScenario()
    : _numDefenders(0)
    , _totalDefense(0)
    , _numChillers(0)
    , _totalChill(0)
    , _modifications(0)
    , _largestChiller(0)
    , _largestDefender(0)
    , _totalDefenseBought(0)
{
    std::fill(_chillHistogram.begin(), _chillHistogram.end(), 0);
    std::fill(_defenseHistogram.begin(), _defenseHistogram.end(), 0);
}

ChillScenario::ChillScenario(const GameState & state, const PlayerID & chillPlayer)
    : _numDefenders(0)
    , _totalDefense(0)
    , _numChillers(0)
    , _totalChill(0)
    , _modifications(0)
    , _largestChiller(0)
    , _largestDefender(0)
    , _totalDefenseBought(0)
{
    std::fill(_chillHistogram.begin(), _chillHistogram.end(), 0);
    std::fill(_defenseHistogram.begin(), _defenseHistogram.end(), 0);

    const PlayerID defender = state.getEnemy(chillPlayer);
    
    for (const auto & cardID : state.getCardIDs(chillPlayer))
    {
        addChiller(state.getCardByID(cardID));
    }

    for (const auto & cardID : state.getCardIDs(defender))
    {
        addDefender(state.getCardByID(cardID));
    }
}

const Histogram & ChillScenario::getDefenseHistogram() const
{
    return _defenseHistogram;
}

const Histogram & ChillScenario::getChillHistogram() const
{
    return _chillHistogram;
}

const HealthType & ChillScenario::getTotalDefense() const
{
    return _totalDefense;
}

const HealthType & ChillScenario::getTotalChill() const
{
    return _totalChill;
}

const HealthType & ChillScenario::getTotalDefenseBought() const
{
    return _totalDefenseBought;
}

const size_t & ChillScenario::getModifications() const
{
    return _modifications;
}

const size_t & ChillScenario::getChillers(const size_t & chill) const
{
    return _chillHistogram[chill];
}

const size_t & ChillScenario::getDefenders(const size_t & health) const
{
    return _defenseHistogram[health];
}

void ChillScenario::addChiller(const HealthType & chill, const size_t n)
{
    if (n == 0)
    {
        return;
    }

    if (chill >= (int)_chillHistogram.size())
    {
        _chillHistogram.resize(chill + 1, 0);
    }

    _chillHistogram[chill] += n;  
    _totalChill += (HealthType)n*chill;
    _numChillers += n;
    _smallestChiller = std::min(chill, _smallestChiller);
    _largestChiller = std::max(chill, _largestChiller);
    _modifications++;
}

void ChillScenario::addChiller(const CardType & type, const size_t n)
{
    if (type.hasTargetAbility() && (type.getTargetAbilityType() == ActionTypes::CHILL))
    {
        addChiller(type.getTargetAbilityAmount(), n);
    }
}

// note: we do not check for action legality here, just assuming the card can use the ability
void ChillScenario::addChiller(const Card & card, const size_t n)
{
    if (!card.isUnderConstruction() && !card.isDelayed())
    {
        addChiller(card.getType(), n);
    }
}

void ChillScenario::removeChiller(const HealthType & chill, const size_t n)
{
    if (n == 0)
    {
        return;
    }

    _modifications++;

    PRISMATA_ASSERT(_chillHistogram[chill] >= n, "Trying to remove a chiller that doesn't exist");
    _chillHistogram[chill] -= n;  

    PRISMATA_ASSERT(_totalChill >= n*chill, "Trying to remove chill from a lesser total");
    _totalChill -= (HealthType)n*chill;

    PRISMATA_ASSERT(_numChillers >= n, "Trying to remove chiller that doesn't exist");
    _numChillers -= n;

    // if this is the sole smallest we need to find the new smallest
    if (chill == _smallestChiller && _chillHistogram[chill] == 0)
    {
        if (_numChillers == 0)
        {
            _smallestChiller = 0;
        }
        else
        {
            for (HealthType i(_smallestChiller); i < _chillHistogram.size(); ++i)
            {
                if (_chillHistogram[i] > 0)
                {
                    _smallestChiller = i;
                    break;
                }
            }
        }
    }

    // if this is the sole largest defender we need to update
    if (chill == _largestChiller && _defenseHistogram[chill] == 0)
    {
        if (_numChillers == 0)
        {
            _largestChiller = 0;
        }
        else
        {
            for (HealthType i(_largestChiller); i > 0; --i)
            {
                if (_chillHistogram[i] > 0)
                {
                    _largestChiller = i;
                    break;
                }
            }
        }
    }
}

void ChillScenario::removeChiller(const CardType & type, const size_t n)
{
    if (type.hasTargetAbility() && (type.getTargetAbilityType() == ActionTypes::CHILL))
    {
        removeChiller(type.getTargetAbilityAmount(), n);
    }
}

void ChillScenario::removeChiller(const Card & card, const size_t n)
{
    removeChiller(card.getType(), n);
}

void ChillScenario::addDefender(const HealthType & health, const size_t n)
{
    if (n == 0)
    {
        return;
    }

    if (health >= _defenseHistogram.size())
    {
        _defenseHistogram.resize(health+1, 0);
    }

    _defenseHistogram[health] += n;
    _totalDefense += (HealthType)n*health;
    _numDefenders += n;
    _smallestDefender = std::min(health, _smallestDefender);
    _largestDefender = std::max(health, _largestDefender);
    _modifications++;
}

void ChillScenario::addDefender(const CardType & type, const size_t n)
{
    if (type.canBlock(false))
    {
        addDefender(type.getStartingHealth(), n);
    }
}

void ChillScenario::addDefender(const Card & card, const size_t n)
{
    if (card.canBlock())
    {
        addDefender(card.currentHealth(), n);
    }
}

void ChillScenario::buyDefender(const CardType & type)
{
    if (type.isPromptBlocker())
    {
        addDefender(type.getStartingHealth());
        _totalDefenseBought += type.getStartingHealth();
    }

    // add any prompt blocking cards created by this card's buy script
    const std::vector<CreateDescription> & created = type.getBuyScript().getEffect().getCreate();
    for (size_t j(0); j < created.size(); ++j)
    {
        if (created[j].getOwn())
        {
            if (created[j].getType().isPromptBlocker())
            {
                addDefender(created[j].getType(), created[j].getMultiple());
                _totalDefenseBought += created[j].getType().getStartingHealth() * (HealthType)created[j].getMultiple();
            }
        }
    }
}

void ChillScenario::sellDefender(const CardType & type)
{
    if (type.isPromptBlocker())
    {
        removeDefender(type.getStartingHealth());
        _totalDefenseBought -= type.getStartingHealth();
    }

    // add any prompt blocking cards created by this card's buy script
    const std::vector<CreateDescription> & created = type.getBuyScript().getEffect().getCreate();
    for (size_t j(0); j < created.size(); ++j)
    {
        if (created[j].getOwn())
        {
            if (created[j].getType().isPromptBlocker())
            {
                removeDefender(created[j].getType(), created[j].getMultiple());
                _totalDefenseBought -= created[j].getType().getStartingHealth() * (HealthType)created[j].getMultiple();
            }
        }
    }
}

void ChillScenario::removeDefender(const HealthType & health, const size_t n)
{
    if (n == 0)
    {
        return;
    }

    _modifications++;

    PRISMATA_ASSERT(_defenseHistogram[health] >= n, "Trying to remove a defender that doesn't exist");
    _defenseHistogram[health] -= n;  

    PRISMATA_ASSERT(_totalDefense >= n * health, "Trying to remove defender from a lesser total");
    _totalDefense -= (HealthType)n*health;

    PRISMATA_ASSERT(_numDefenders >= n, "Trying to remove defender that doesn't exist");
    _numDefenders -= n;

    // if this is the sole smallest we need to find the new smallest
    if (health == _smallestDefender && _defenseHistogram[health] == 0)
    {
        if (_numDefenders == 0)
        {
            _smallestDefender = 0;
        }
        else
        {
            for (HealthType i(_smallestDefender); i < _defenseHistogram.size(); ++i)
            {
                if (_defenseHistogram[i] > 0)
                {
                    _smallestDefender = i;
                    break;
                }
            }
        }
    }

    // if this is the sole largest defender we need to update
    if (health == _largestDefender && _defenseHistogram[health] == 0)
    {
        if (_numDefenders == 0)
        {
            _largestDefender = 0;
        }
        else
        {
            for (HealthType i(_largestDefender); i > 0; --i)
            {
                if (_defenseHistogram[i] > 0)
                {
                    _largestDefender = i;
                    break;
                }
            }
        }
    }
}

void ChillScenario::removeDefender(const CardType & type, const size_t n)
{
    if (type.canBlock(false))
    {
        removeDefender(type.getStartingHealth(), n);
    }
}

void ChillScenario::removeDefender(const Card & card, const size_t n)
{
    if (card.canBlock())
    {
        removeDefender(card.currentHealth(), n);
    }
}

const size_t & ChillScenario::getNumChillers() const
{
    return _numChillers;
}

const size_t & ChillScenario::getNumDefenders() const
{
    return _numDefenders;
}

void ChillScenario::setRandomData(const size_t & histogramMinIndex, const size_t & histogramMaxIndex, const size_t & maxHistogramValue)
{
    (*this) = ChillScenario();

    for (size_t c(histogramMinIndex); c < histogramMaxIndex; ++c)
    {
        int r = rand() % maxHistogramValue;

        for (int i(0); i < r; ++i)
        {
            addChiller((HealthType)c);
        }
    }

    for (size_t c(histogramMinIndex); c < histogramMaxIndex; ++c)
    {
        int r = rand() % maxHistogramValue;
        
        for (int i(0); i < r; ++i)
        {
            addDefender((HealthType)c);
        }
    }
}

void ChillScenario::print() const
{
    printf("Chill: [");
    for (size_t c(1); c < _chillHistogram.size(); ++c)
    {
        printf("%d%s", _chillHistogram[c], c < _chillHistogram.size() - 1 ? "," : "");
    }

    printf("]  Defense: [");
    for (size_t c(1); c < _defenseHistogram.size(); ++c)
    {
        printf("%d%s", _defenseHistogram[c], c < _defenseHistogram.size() - 1 ? "," : "");
    }

    printf("]\n");
}

const HealthType & ChillScenario::getLargestDefender() const
{
    return _largestDefender;
}

const HealthType & ChillScenario::getLargestChiller() const
{
    return _largestChiller;
}

HealthType ChillScenario::calculateHeuristicUsedChill()
{
    // we can't chill nuttin'
    if (_numChillers == 0 || _numDefenders == 0)
    {
        return 0;
    }

    // if all our chillers are bigger than all their defenders then we can chill all
    if ((_numChillers >= _numDefenders) && (_smallestChiller >= _largestDefender))
    {
        return _totalDefense;
    }

    std::vector<size_t> newChillHistogram(_chillHistogram.size(), 0);
    std::vector<size_t> newDefenseHistogram(_defenseHistogram.size(), 0);

    HealthType topDownwastedChill = calculateTopDownChillWastedHeuristic(newChillHistogram, newDefenseHistogram);

    HealthType wastedChill = calculateTotalsChillWastedHeuristic(newChillHistogram, newDefenseHistogram);
    wastedChill = std::max(wastedChill, calculateTwosChillWastedHeuristic(newChillHistogram, newDefenseHistogram));
    wastedChill = std::max(wastedChill, calculateThreesChillWastedHeuristic(newChillHistogram, newDefenseHistogram));
    
    PRISMATA_ASSERT(_totalChill >= wastedChill + topDownwastedChill, "Blah");

    // return the max of them
    return _totalChill - (wastedChill + topDownwastedChill);
}

HealthType ChillScenario::calculateTopDownChillWastedHeuristic(std::vector<size_t> & newChillHistogram, std::vector<size_t> & newDefenseHistogram) const
{
    newChillHistogram.assign(_chillHistogram.begin(), _chillHistogram.end());
    newDefenseHistogram.assign(_defenseHistogram.begin(), _defenseHistogram.end());

    if (_largestChiller < _largestDefender)
    {
        return 0;
    }

    HealthType wastedChill = 0;
    HealthType chillIndex = _largestChiller;
    HealthType defenseIndex = _largestDefender;
    HealthType chillUsed = 0;
    HealthType defenseUsed = 0;

    while (chillIndex > 0 && chillIndex >= defenseIndex)
    {
        HealthType numChill = (HealthType)_chillHistogram[chillIndex] - chillUsed;
        HealthType numDef = (HealthType)_defenseHistogram[defenseIndex] - defenseUsed;

        HealthType take = std::min(numChill, numDef);

        chillUsed += take;
        defenseUsed += take;
        wastedChill += take * (chillIndex - defenseIndex);

        newChillHistogram[chillIndex] -= take;
        newDefenseHistogram[defenseIndex] -= take;

        if (chillUsed == _chillHistogram[chillIndex])
        {
            chillUsed = 0;
            chillIndex--;
        }

        if (defenseUsed == _defenseHistogram[defenseIndex])
        {
            defenseUsed = 0;
            defenseIndex--;
        }
    }

    return wastedChill;
}



HealthType ChillScenario::calculateTotalsChillWastedHeuristic(std::vector<size_t> & chillHistogram, std::vector<size_t> & defenseHistogram) const
{
    HealthType totalChill = 0;
    HealthType totalDefense = 0;

    for (HealthType i(_smallestChiller); i <= _largestChiller; ++i)
    {
        totalChill += i * (HealthType)chillHistogram[i];
    }    

    for (HealthType i(_smallestDefender); i <= _largestDefender; ++i)
    {
        totalDefense += i * (HealthType)defenseHistogram[i];
    }

    return (totalChill > totalDefense) ? (totalChill - totalDefense) : 0;
}

HealthType ChillScenario::calculateTwosChillWastedHeuristic(std::vector<size_t> & chillHistogram, std::vector<size_t> & defenseHistogram) const
{
    size_t chillParitySum[2] = {0, 0};
    size_t defenseParitySum[2] = {0, 0};
    size_t chillParityCount[2] = {0, 0};
    size_t defenseParityCount[2] = {0, 0};

    for (size_t i(_smallestChiller); i <= _largestChiller; ++i)
    {
        size_t parity = i & 1;
        chillParitySum[parity] += i * chillHistogram[i];
        chillParityCount[parity] += chillHistogram[i];
    }    

    for (size_t i(_smallestDefender); i <= _largestDefender; ++i)
    {
        size_t parity = i & 1;
        defenseParitySum[parity] += i * defenseHistogram[i];
        defenseParityCount[parity] += defenseHistogram[i];
    }

    size_t chillTwos = (chillParitySum[0] + chillParitySum[1] - chillParityCount[1]) / 2;
    size_t defTwos = (defenseParitySum[0] + defenseParitySum[1] - defenseParityCount[1]) / 2;
    size_t subTwos = std::min(defTwos, chillTwos);
    chillTwos -= subTwos;
    defTwos -= subTwos;
    size_t defOnesRemaining = defenseParityCount[1] - chillParityCount[1];
    size_t waste = chillTwos;
    if (chillTwos > defOnesRemaining) { waste += (chillTwos - defOnesRemaining); }

    // if it's all evens vs. all odds it's at least 1
    if (waste == 0 && chillParityCount[0] > 0 && chillParityCount[1] == 0 && defenseParityCount[0] == 0 && defenseParityCount[1] > 0)
    {
        waste = 1;
    }

    return (HealthType)waste;
}

HealthType ChillScenario::calculateThreesChillWastedHeuristic(std::vector<size_t> & chillHistogram, std::vector<size_t> & defenseHistogram) const
{
    HealthType chillDivThreeSum = 0;
    HealthType defenseDivThreeSum = 0;
    HealthType defenseNotDivThreeCount = 0;

    for (HealthType i(_smallestChiller); i <= _largestChiller; ++i)
    {
        chillDivThreeSum += (i / 3) * (HealthType)chillHistogram[i];
    }    

    for (HealthType i(_smallestDefender); i <= _largestDefender; ++i)
    {
        defenseDivThreeSum += (i / 3) * (HealthType)defenseHistogram[i];
        defenseNotDivThreeCount += ((i % 3) == 0) ? 1 : (HealthType)defenseHistogram[i];
    }

    HealthType chillThrees = chillDivThreeSum;
    HealthType defThrees = defenseDivThreeSum;
    HealthType subThrees = std::min(chillThrees, defThrees);
    chillThrees -= subThrees;
    defThrees -= subThrees;
    HealthType defOnesRemaining = defenseNotDivThreeCount;
    HealthType waste = chillThrees;
    if (chillThrees > defOnesRemaining) { waste += 2*(chillThrees - defOnesRemaining); }
    return waste;
}

#include "ChillIterator.h"
HealthType ChillScenario::calculateUsedChill(const size_t & maxSolverIterations)
{
    ChillIterator chillIterator(*this);
    chillIterator.solve(maxSolverIterations);

    // if we perfectly solved it, return the solution
    if (chillIterator.isSolved())
    {
        return chillIterator.getBestUsedChill();
    }
        
    // otherwise take the max of the best found solution and the fast heuristic
    return std::max(chillIterator.getBestUsedChill(), calculateHeuristicUsedChill());
}

HealthType ChillScenario::calculateExactUsedChill() const
{
    ChillIterator chillIterator(*this);
    chillIterator.solve();
    return chillIterator.getBestUsedChill();
}

GameState ChillScenario::constructGameState() const
{
    GameState state;

    const std::vector<CardType> & allCardTypes = CardTypes::GetAllCardTypes();

    for (size_t i(1); i < _chillHistogram.size(); ++i)
    {
        for (size_t c(0); c < allCardTypes.size(); ++c)
        {
            const CardType & type = allCardTypes[c];

            if (type.hasTargetAbility() && (type.getTargetAbilityType() == ActionTypes::CHILL) && (type.getTargetAbilityAmount() == i))
            {
                state.addCard(Players::Player_One, type, _chillHistogram[i], CardCreationMethod::Manual, 0, 0);
                break;
            }
        }
    }

    for (size_t i(1); i < _defenseHistogram.size(); ++i)
    {
        for (size_t c(0); c < allCardTypes.size(); ++c)
        {
            const CardType & type = allCardTypes[c];

            if (type.canBlock(false) && type.getStartingHealth() == i)
            {
                state.addCard(Players::Player_Two, type, _defenseHistogram[i], CardCreationMethod::Manual, 0, 0);
                break;
            }
        }
    }

    return state;
}