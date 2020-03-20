#include "DefenseHistogramExperiment.h"

using namespace Prismata;

DefenseHistogramExperiment::DefenseHistogramExperiment(PlayerPtr p1, PlayerPtr p2, const std::vector<size_t> & histogram)
    : _defenseHistogram(histogram)
    , _samples(0)
    , _samplesPositive(0)
{
    _players[0] = p1;
    _players[1] = p2;

    _droneType = CardTypes::GetCardType("Drone");
}

void DefenseHistogramExperiment::run(const size_t & games)
{
    _samples = 0;
    _samplesPositive = 0;
    _defenseCounts = vvs(_defenseHistogram.size(), std::vector<size_t>(20, 0));
    _defenseNegativeCounts = std::vector<size_t>(_defenseHistogram.size(), 0);

    const size_t numDominionCards = 8;

    for (size_t g(0); g < games; ++g)
    {
        GameState state;
        state.setStartingState(Players::Player_One, numDominionCards);

        Game game(state, _players[0]->clone(), _players[1]->clone());
        parseState(game.getState());

        // manually play out the game so we can collect the data
        size_t turns = 0;
        while(!game.gameOver())
        {
            ++turns;
            game.playNextTurn();

            parseState(game.getState());
        }

        if (g%10 == 0)
        {
            printf("Played %d games\n", g);
        }
    }
}

void DefenseHistogramExperiment::parseState(const GameState & state)
{
    std::vector<size_t> defenseCount(_defenseHistogram.size(), 0);
    std::fill(std::begin(defenseCount), std::end(defenseCount), 0);

    // count the occurrences of defenders
    for (PlayerID p(0); p < 2; ++p)
    {
        for (const auto & cardID : state.getCardIDs(p))
        {
            const Card & card = state.getCardByID(cardID);

            if (card.getType() == _droneType)
            {
                continue;
            }

            if (card.canBlock() && card.currentHealth() < (int)defenseCount.size())
            {
                defenseCount[card.currentHealth()]++;
            }
        }
    }

    // parse the data
    bool samplePositive = true;
    for (size_t i(0); i < defenseCount.size(); ++i)
    {
        if (defenseCount[i] < _defenseCounts[i].size())
        {
            if (defenseCount[i] > 0)
            {
                _defenseCounts[i][defenseCount[i]]++;
            }
        }

        if (defenseCount[i] > _defenseHistogram[i])
        {
            _defenseNegativeCounts[i]++;
            samplePositive = false;
            break;
        }
    }

    if (samplePositive)
    {
        _samplesPositive++;
    }

    _samples++;
}

void DefenseHistogramExperiment::printResults()
{
    printf("\n\n      ");
    for (size_t j(0); j < _defenseCounts[0].size(); ++j)
    {
        printf("%6d", j);
    }
    printf("\n");

    for (size_t i(0); i < _defenseCounts.size(); ++i)
    {
        printf("%3d : ", i);

        for (size_t j(0); j < _defenseCounts[i].size(); ++j)
        {
            printf("%6.2lf", 100*(double)_defenseCounts[i][j]/_samples);
        }

        printf("\n");
    }

    vvs cdc(_defenseCounts);

    printf("\n\n      ");
    for (size_t j(0); j < _defenseCounts[0].size(); ++j)
    {
        printf("%6d", j);
    }
    printf("\n");
    for (size_t i(0); i < cdc.size(); ++i)
    {
        for (size_t j(0); j < cdc[i].size(); ++j)
        {
            for (size_t k(0); k < j; ++k)
            {
                cdc[i][j] += _defenseCounts[i][k];
            }
        }
    }

    for (size_t i(0); i < cdc.size(); ++i)
    {
        printf("%3d : ", i);

        for (size_t j(0); j < cdc[i].size(); ++j)
        {
            printf("%6.2lf", 100*(double)cdc[i][j]/_samples);
        }

        printf("\n");
    }

    printf("\n\n%d positve of %d samples\n\n", _samplesPositive, _samples);
    for (size_t i(0); i < _defenseNegativeCounts.size(); ++i)
    {
        printf("%3d Failures (Max %d): %5d\n", i, _defenseHistogram[i], _defenseNegativeCounts[i]);
    }

    printf("\n");
}