#include "ChillIterator.h"
#include "Timer.h"

using namespace Prismata;

const int MAX_NODES_REACHED = -1;

ChillIterator::ChillIterator(const ChillScenario & scenario)
    : _nodesSearched(0)
    , _bestChillUsedLowerBound(0)
    , _bestUsedChill(0)
    , _bestWastedChill(0)
    , _maxNodes(0)
    , _chillScenario(scenario)
    , _solved(false)
    , _isReset(true)
    , _bestSolutionMaxRemainingDefender(0)
    , _bestSolutionCount(0)
    , _remainingChillHistogram(_chillScenario.getChillHistogram())
    , _remainingDefenderHistogram(_chillScenario.getDefenseHistogram())
{
    _bestChillUsedLowerBound = calculateChillUsedLowerBound();
    _bestUsedChill = _bestChillUsedLowerBound;
}

void ChillIterator::reset()
{
    if (_isReset)
    {
        return;
    }

    _solved = false;
    _nodesSearched = 0;
    _maxNodes = 0;
    _bestUsedChill = _bestChillUsedLowerBound;
    _bestWastedChill = 0;
    _bestSolutionMaxRemainingDefender = 0;
    _bestSolutionCount = 0;
    _remainingChillHistogram = _chillScenario.getChillHistogram();
    _remainingDefenderHistogram = _chillScenario.getDefenseHistogram();
    _isReset = true;
}

void ChillIterator::solve(const size_t maxNodes)
{
    reset();
    _isReset = false;
    _maxNodes = maxNodes;    

    try
    {
        recurse(0, 0, 0, 0);
        _solved = true;
    }
    catch (int e)
    {
        if (e == MAX_NODES_REACHED)
        {
            _solved = false;
        }
    }
}

bool ChillIterator::isSolved() const
{
    return _solved;
}

const size_t & ChillIterator::getNodesSearched() const
{
    return _nodesSearched;
}

void ChillIterator::debugSolve(const size_t maxNodes)
{
    reset();
    _maxNodes = maxNodes;

    Timer t;
    t.start();
    try
    {
        recurse(0, 0, 0, 0);
        _solved = true;
    }
    catch (int e)
    {
        if (e == MAX_NODES_REACHED)
        {
            _solved = false;
        }
    }
    double ms = t.getElapsedTimeInMilliSec();

    printf("%d Nodes Searched in %lfms @ %lf nodes/sec\n", _nodesSearched, ms, _nodesSearched/ms*1000);
    printf("MaxUsedChill = %d, WastedChill = %d, TotalChill = %d, TotalDef = %d\n", _bestUsedChill, _bestWastedChill, _chillScenario.getTotalChill(), _chillScenario.getTotalDefense());    
    //printStack(_bestActionStack, _bestUsedChill, _bestWastedChill);
    printf("\n");
}

// accounts for chiller and blocker isomorphisms
void ChillIterator::recurse(HealthType totalUsedChill, HealthType totalWastedChill, HealthType currentBlockerIndex, HealthType partialDefenderChill)
{
    ++_nodesSearched;
    
    // check for a new best solution
    if (totalUsedChill == _bestUsedChill)
    {
        if (!histogramsEqual(_remainingDefenderHistogram, _bestSolutionRemainingDefenderHistogram))
        {
            _bestSolutionCount++;
        }
    }

    if ((totalUsedChill > _bestUsedChill) || ((totalUsedChill == _bestUsedChill) && totalWastedChill < _bestWastedChill))
    {
        _bestSolutionCount = 1;
        _bestUsedChill = totalUsedChill;
        _bestWastedChill = totalWastedChill;
        _bestSolutionRemainingDefenderHistogram = _remainingDefenderHistogram;
        //_bestActionStack = _actionStack;

        for (HealthType i = _chillScenario.getLargestDefender(); i > 0; --i)
        {
            if (_remainingDefenderHistogram[i] > 0)
            {
                _bestSolutionMaxRemainingDefender = i;
                break;
            }
        }
    }

    // prune if we can never possibly reach the best we've found so far
    if (_chillScenario.getTotalChill() - totalWastedChill <= _bestUsedChill)
    {
        return;
    }

    // prune if we're strictly worse than the best so far
    // this might be a subset of the previous check but leave it in for now
    if (totalWastedChill > _bestWastedChill)
    {
        return;
    }

    if (_maxNodes > 0 && _nodesSearched >= _maxNodes)
    {
        throw MAX_NODES_REACHED;
    }
    
    //printStack(_actionStack, totalUsedChill, totalWastedChill);

    HealthType defenderIndex = currentBlockerIndex;
    HealthType defenderIndexMax = (HealthType)_chillScenario.getLargestDefender();

    if (defenderIndexMax > 0)
    {
        defenderIndexMax += 1;
    }
        
    // if we have partially applied chill to a unit, we must finish freezing that unit before moving on
    // we could add a special case for this or just set the move iterator to only allow for this move
    if (partialDefenderChill > 0)
    {
        defenderIndex = currentBlockerIndex;
        defenderIndexMax = defenderIndex + 1; 
    }

    // iterate first through every remaining defender health amount
    for (; defenderIndex < defenderIndexMax; ++defenderIndex)
    {
        // we have no remaining blockers of this size
        if (_remainingDefenderHistogram[defenderIndex] == 0)
        {
            continue;
        }

        HealthType chillerIndex = 1;
        HealthType chillerIndexMax = (HealthType)_chillScenario.getLargestChiller();

        // shalev's optimzation: only iterate up to the first chiller with amount >= this units size
        HealthType healthRemaining = defenderIndex - partialDefenderChill;
        for (HealthType cIndex(healthRemaining); cIndex < _remainingChillHistogram.size(); ++cIndex)
        {
            if (_remainingChillHistogram[cIndex] > 0)
            {
                chillerIndexMax = cIndex + 1;
                break;
            }
        }

        // iterate over choice of chiller to chill this unit with
        for (; chillerIndex < chillerIndexMax; ++chillerIndex)
        {
            const HealthType chillAmount = chillerIndex;

            // if we don't have any chillers of this size remaining then continue
            if (_remainingChillHistogram[chillerIndex] == 0)
            {
                continue;
            }

            HealthType defenderChilledFor = partialDefenderChill + chillAmount;

            // remove this chiller from the histogram so we don't re-use it
            _remainingChillHistogram[chillerIndex]--;
            //_actionStack.push_back(Action(0, 0, chillAmount, defenderIndex));

            // if we froze the unit
            if (defenderChilledFor >= defenderIndex)
            {
                _remainingDefenderHistogram[defenderIndex]--;

                // if we froze the unit entirely, then we can recurse such that we can freely choose our next target
                recurse(totalUsedChill + defenderIndex, totalWastedChill + defenderChilledFor - defenderIndex, currentBlockerIndex, 0);

                _remainingDefenderHistogram[defenderIndex]++;
            }
            // partial freeze only
            else
            {
                // if we only partially froze the target, pass on the relevant information to the next recursive step
                recurse(totalUsedChill, totalWastedChill, defenderIndex, defenderChilledFor);
            }
            
            _remainingChillHistogram[chillerIndex]++;
            //_actionStack.pop_back();
        }
    }    
}

void ChillIterator::printStack(const std::vector<Action> & actionStack, HealthType usedChill, HealthType wastedChill)
{
    printf("%3d %3d ", usedChill, wastedChill);

    for (size_t i(0); i < actionStack.size(); ++i)
    {
        printf("(%d,%d)  ", actionStack[i].getID(), actionStack[i].getTargetID());
    }

    printf("\n");
}

const HealthType & ChillIterator::getBestUsedChill() const
{
    return _bestUsedChill;
}

const HealthType & ChillIterator::getBestWastedChill() const
{
    return _bestWastedChill;
}

void ChillIterator::printRemainingHistograms() const
{
    printf("Remaining Chill: [");
    for (size_t c(1); c < _remainingChillHistogram.size(); ++c)
    {
        printf("%d%s", _remainingChillHistogram[c], c < _remainingChillHistogram.size() - 1 ? "," : "");
    }

    printf("]\nRemaining Defense: [");
    for (size_t c(1); c < _remainingDefenderHistogram.size(); ++c)
    {
        printf("%d%s", _remainingDefenderHistogram[c], c < _remainingDefenderHistogram.size() - 1 ? "," : "");
    }

    printf("]\n");
}

const size_t & ChillIterator::getBestSolutionCount() const
{
    return _bestSolutionCount;
}

bool ChillIterator::histogramsEqual(const Histogram & h1, const Histogram & h2) const
{
    if (h1.size() != h2.size())
    {
        return false;
    }

    for (size_t i(0); i < h1.size(); ++i)
    {
        if (h1[i] != h2[i])
        {
            return false;
        }
    }

    return true;
}

HealthType ChillIterator::calculateChillUsedLowerBound() const
{
    Histogram chillHistogram(_remainingChillHistogram);
    Histogram defenseHistogram(_remainingDefenderHistogram);

    HealthType totalChillRemaining = _chillScenario.getTotalChill();
    HealthType totalChillApplied = 0;
    HealthType totalWastedChill = 0;
    HealthType defenseIndex = std::min(_chillScenario.getTotalChill(), _chillScenario.getLargestDefender());

    // first go through and remove all perfect matches
    // tests show this actually performs slower
    /*for (size_t c(1); c < chillHistogram.size() && c < defenseHistogram.size(); ++c)
    {
        size_t take = std::min(chillHistogram[c], defenseHistogram[c]);
        totalChillRemaining -= c * take;
        totalChillApplied += c * take;
        chillHistogram[c] -= take;
        defenseHistogram[c] -= take;
    }*/
    
    while (defenseIndex > 0)
    {
        // if the defender doesn't exist, skip it
        if (defenseHistogram[defenseIndex] == 0)
        {
            defenseIndex--;
            continue;
        }

        // if we can't possibly freeze this card then skip it
        if (totalChillRemaining < defenseIndex)
        {
            defenseIndex--;
            continue;
        }

        HealthType chillApplied = 0;

        // keep applying chillers until it's frozen
        while (chillApplied < defenseIndex)
        {
            // choose the chiller to be applied to this defender
            HealthType chillRemaining = defenseIndex - chillApplied;
            HealthType bestChiller = 0;
           
            // find the smallest chiller above the remainder
            for (size_t c(chillRemaining); (bestChiller == 0) && (c <= _chillScenario.getLargestChiller()); ++c)
            {
                if (chillHistogram[c] > 0)
                {
                    bestChiller = (HealthType)c;
                }
            }
           
            // find the largest chiller below the remainder
            size_t c = chillRemaining;
            if (c >= chillHistogram.size() && c > 0)
            {
                c = chillHistogram.size() - 1;
            }

            for (; (bestChiller == 0) && (c > 0); --c)
            {
                if (chillHistogram[c] > 0)
                {
                    bestChiller = (HealthType)c;
                }
            }

            chillHistogram[bestChiller]--;
            chillApplied += bestChiller;
        }
        
        totalChillRemaining -= chillApplied;
        totalChillApplied += defenseIndex;
        totalWastedChill += (chillApplied - defenseIndex);
        defenseHistogram[defenseIndex]--;
    }

    return totalChillApplied;
}