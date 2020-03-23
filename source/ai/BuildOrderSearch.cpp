#include "BuildOrderSearch.h"

using namespace Prismata;

BuildOrderSearch::BuildOrderSearch(const BuildOrderSearchParameters & params)
    : _numBuys(0)
    , _nodesSearched(0)
    , _cardTypeCount(_params.getInitialState().numCardsBuyable(), 0)
    , _buyPlayer(params.getInitialState().getActivePlayer())
    , _params(params)
    , _currentMaxTurn(0)
    , _bestSolutionMaxDrones(0)
{
     _allowedBuyableIndex.reserve(_params.getInitialState().numCardsBuyable());
    _boughtCardIDStack.reserve(20);
    
    _droneType = CardTypes::GetCardType("Drone");

    const GameState & state = _params.getInitialState();

    for (size_t i(0); i < state.numCardsBuyable(); ++i)
    {
        // set up the number of cards of each type we already have
        _cardTypeCount[i] = state.numCardsOfType(state.getActivePlayer(), state.getCardBuyableByIndex(i).getType());

        if (params.isRelevant(state.getCardBuyableByIndex(i).getType()))
        {
            _allowedBuyableIndex.push_back(i);
        }
    }

    PRISMATA_ASSERT(_params.getInitialState().numCards(_params.getInitialState().getInactivePlayer()) == 0, "Enemy player in the state must have no units, this is a single agent problem");
}

#include "Timer.h"
void BuildOrderSearch::solve()
{
    _numBuys = 0;

    Timer timer;
    timer.start();

    printf("\n\n   MaxTurn        TimeMS         Nodes     Nodes/Sec      Solution\n");
    printf("------------------------------------------------------------------\n");

    try
    {
        // iterative deepening
        for (size_t t(0); t < 100; ++t)
        {
            GameState state(_params.getInitialState());
            _currentMaxTurn = t;

            _econMove.clear();
            _buyPlayer.getMove(state, _econMove);
            recurse(state, 0, 0, 0);

            double ms = timer.getElapsedTimeInMilliSec();
            double nps = _nodesSearched / ms * 1000;

            if (_solutionFound)
            {
                printf("\n\n%10d%14.2lf%14d%14.2lf%14s\n", _currentMaxTurn+1, ms, _nodesSearched, nps, (_solutionFound ? "true" : "false"));
                printf("\nBest Solution Found:\n\n%s\n", _bestSolutionString.c_str());
                break;
            }
            else
            {
                printf("%10d%14.2lf%14d%14.2lf%14s\n", _currentMaxTurn+1, ms, _nodesSearched, nps, (_solutionFound ? "true" : "false"));
            }
        }
    }
    catch (int e)
    {
        e++;
        _solutionFound = true;
        double ms = timer.getElapsedTimeInMilliSec();
        double nps = _nodesSearched / ms * 1000;
        printf("%10d%14.2lf%14d%14.2lf%14s\n", _currentMaxTurn, ms, _nodesSearched, nps, (_solutionFound ? "true" : "false"));
        
    }
}

void BuildOrderSearch::passTurn(const GameState & state, const size_t turn)
{
    GameState nextState(state);

    const Action pass(state.getActivePlayer(), ActionTypes::END_PHASE, 0);
    const Action enemyPass(state.getInactivePlayer(), ActionTypes::END_PHASE, 0);

    // we are in the action phase with no breach targets, so passing twice goes to enemy turn
    nextState.doAction(pass);
    nextState.doAction(pass);

    // enemy has no defenders so we are in action phase, pass twice goes back to buy turn
    nextState.doAction(enemyPass);
    nextState.doAction(enemyPass);

    // activate all economy cards on the purchasing player's side
    _econMove.clear();
    _buyPlayer.getMove(nextState, _econMove);

    recurse(nextState, 0, 0, turn + 1);
}

void BuildOrderSearch::updateSolution(const GameState & state, const size_t & turn)
{
    if (_params.getGoal().meetsGoal(state))
    {
        if (!_solutionFound || (state.numCardsOfType(0, _droneType) > _bestSolutionMaxDrones))
        {
            _solutionFound = true;
            _bestSolutionTurn = turn;
            _bestSolutionString = getStackString(turn);
            _bestSolutionMaxDrones = state.numCardsOfType(0, _droneType);

            printf("\nSolution Found with %d Drones\n\n%s", _bestSolutionMaxDrones, _bestSolutionString.c_str());
        }
    }
}

bool BuildOrderSearch::isTerminalNode(const GameState & state, const CardID currentCardBuyableIndex, const size_t & turn)
{
    if (currentCardBuyableIndex >= _allowedBuyableIndex.size())
    {
        if (!_actionStack.empty())
        {
            ++_numBuys;
        }

        return true;
    }

    if (turn > _currentMaxTurn)
    {
        return true;
    }

    if (_solutionFound && turn > _bestSolutionTurn)
    {
        return true;
    }

    if (_params.getGoal().cannotMeetGoal(state))
    {
        return true;
    }

    return false;
}

void BuildOrderSearch::recurse(GameState & state, const CardID currentCardBuyableIndex, const size_t numBought, const size_t turn)
{
    ++_nodesSearched;

    if (_actionStack.size() <= turn)
    {
        _actionStack.push_back(std::vector<Action>());
        _actionStack[turn].reserve(20);
    }

    updateSolution(state, turn);

    if (isTerminalNode(state, currentCardBuyableIndex, turn))
    {
        return;
    }

    const CardType cardType = state.getCardBuyableByIndex(_allowedBuyableIndex[currentCardBuyableIndex]).getType();
    const Action buyCard(state.getActivePlayer(), ActionTypes::BUY, cardType.getID());
        
    bool hasBuyLimit = _params.getBuyLimits().hasLimit(cardType.getID());
    CardID numOwned = state.numCardsOfType(0, cardType);
    bool buyLimitExceeded = hasBuyLimit && (numOwned >= _params.getBuyLimits().getLimit(cardType));

    if (state.isLegal(buyCard) && !buyLimitExceeded)
    {
        // buy the card
        state.doAction(buyCard);
        _boughtCardIDStack.push_back(state.getLastCardBoughtID());
        _actionStack[turn].push_back(buyCard);
        _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]]++;
        
        // call recursion after having bought this card
        recurse(state, currentCardBuyableIndex, numBought + 1, turn);

        // try the option of passing the turn after buying this card
        passTurn(state, turn);

        // sell the card
        _actionStack[turn].pop_back();
        _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]]--;

        const Action sell(buyCard.getPlayer(), ActionTypes::SELL, _boughtCardIDStack.back());
        _boughtCardIDStack.pop_back();

        state.doAction(sell);
    }

    // call recursion after having skipped this card
    recurse(state, currentCardBuyableIndex + 1, 0, turn);
}

size_t BuildOrderSearch::getNodesSearched() 
{
    return _nodesSearched;
}

std::string BuildOrderSearch::getStackString(const size_t & turn) const
{
    std::stringstream ss;

    for (size_t i(0); i<=turn; ++i)
    {
        ss << "T" << (i+1) << ": ";
        for (size_t j(0); j < _actionStack[i].size(); ++j)
        {
            ss << ((j > 0) ? ", " : "") << CardTypes::GetAllCardTypes()[_actionStack[i][j].getID()].getUIName();
        }
        
        ss << "\n";
    }

    return ss.str();
}

size_t BuildOrderSearch::getNumBuys()
{
    return _numBuys;
}

void BuildOrderSearch::DoBuildOrderSearch(const rapidjson::Value & val)
{
    PRISMATA_ASSERT(val.IsObject(), "Build order search array member must be object");

    if (val.HasMember("run") && !val["run"].GetBool())
    {
        return;
    }

    std::cout << "Starting Build Order Search: " << val["name"].GetString() << "\n\n";

    Prismata::BuildOrderSearchParameters params(val);
    Prismata::BuildOrderSearch buildOrderSearch(params);

    buildOrderSearch.solve();
}

void BuildOrderSearch::ParseBuildOrderSearch(const std::string & json)
{
    rapidjson::Document document;
    bool parsingFailed = document.Parse(json.c_str()).HasParseError();

    if (parsingFailed)
    {
        std::cout << "Couldn't parse build order search config file";
        return;
    }

    if (document.HasMember("BuildOrderSearch") && document["BuildOrderSearch"].IsArray())
    {
        for (size_t i(0); i < document["BuildOrderSearch"].Size(); ++i)
        {
            DoBuildOrderSearch(document["BuildOrderSearch"][i]);
        }
    }
    else
    {
        std::cout << "No build order search options found\n";
    }
}