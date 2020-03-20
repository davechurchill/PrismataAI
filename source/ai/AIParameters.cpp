#include "AIParameters.h"

using namespace Prismata;

AIParameters::AIParameters()
    : _partialPlayerParses(0)
    , _playerParses(0)
    , _moveIteratorParses(0)
{
    _playerKeyNames         = { "Players", "Experimental Players" };
    _partialPlayerKeyNames  = { "Partial Players", "Experimental Partial Players" };
    _stateKeyNames          = { "States"};
    _moveIteratorKeyNames   = { "Move Iterators" };
    _openingBookKeyNames    = { "Opening Books" };
    _filterKeyNames         = { "Filters" };
    _buyLimitKeyNames       = { "Buy Limits"};
}

AIParameters & AIParameters::Instance()
{
    static AIParameters params;
    return params;
}

void AIParameters::parseJSONValue(const rapidjson::Value & rootValue)
{  
    Instance() = AIParameters();

    PRISMATA_ASSERT(rootValue.HasMember("Partial Players"),  "AIParameters: No 'Partial Players' Options Found");
    PRISMATA_ASSERT(rootValue.HasMember("Move Iterators"),   "AIParameters: No 'Move Iterators' Options Found");
    PRISMATA_ASSERT(rootValue.HasMember("Players"),          "AIParameters: No 'Players' Options Found");

    Timer t;
    t.start();
    
    std::cout << "Parsing Buy Limits...\n";
    for (size_t i(0); i < _buyLimitKeyNames.size(); ++i)
    {
        parseBuyLimits(_buyLimitKeyNames[i], rootValue);
    }

    std::cout << "Parsing Card Filters...\n";
    for (size_t i(0); i < _filterKeyNames.size(); ++i)
    {
        parseFilters(_filterKeyNames[i], rootValue);
    }

    std::cout << "Parsing Opening Books...\n";
    for (size_t i(0); i < _openingBookKeyNames.size(); ++i)
    {
        parseOpeningBooks(_openingBookKeyNames[i], rootValue);
    }

    std::cout << "Parsing Partial Players...\n";
    for (size_t i(0); i < _partialPlayerKeyNames.size(); ++i)
    {
        parsePartialPlayers(_partialPlayerKeyNames[i], rootValue);
    }

    std::cout << "Parsing Move Iterators...\n";
    for (size_t i(0); i < _moveIteratorKeyNames.size(); ++i)
    {
        parseMoveIterators(_moveIteratorKeyNames[i], rootValue);
    }

    std::cout << "Parsing Players...\n";
    for (size_t i(0); i < _playerKeyNames.size(); ++i)
    {
        parsePlayers(_playerKeyNames[i], rootValue);
    }

    std::cout << "Parsing States...\n";
    for (size_t i(0); i < _stateKeyNames.size(); ++i)
    {
        parseStates(_stateKeyNames[i], rootValue);
    }

    // add two human players
    _playerNames.push_back("Human");
    _playerMap[Players::Player_One]["Human"] = PlayerPtr(new Player_GUI(Players::Player_One));
    _playerMap[Players::Player_Two]["Human"] = PlayerPtr(new Player_GUI(Players::Player_Two));

    std::cout << "Parsing Complete!\n";

    double ms = t.getElapsedTimeInMilliSec();

    std::cout << "Opening Books:      " << _openingBookMap[0].size()   << std::endl;
    std::cout << "Partial Players:    " << _partialPlayerMap[0].size() << ", parses: " << _partialPlayerParses << std::endl;
    std::cout << "Move Iterators:     " << _moveIteratorMap[0].size()  << ", parses: " << _moveIteratorParses << std::endl;
    std::cout << "Players:            " << _playerMap[0].size()        << ", parses: " << _playerParses << std::endl;
    std::cout << "States:             " << _stateMap.size()            << std::endl;
    std::cout << "Parse Time:         " << ms << "ms"                  << std::endl;
}

void AIParameters::parseStates(const std::string & keyName, const rapidjson::Value & rootValue)
{
    if (!rootValue.HasMember(keyName.c_str()))
    {
        return;
    }
    
    const rapidjson::Value & filters = rootValue[keyName.c_str()];
    for (rapidjson::Value::ConstMemberIterator itr = filters.MemberBegin(); itr != filters.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
        
        PRISMATA_ASSERT(val.IsObject(), "Filter must be an object");
        PRISMATA_ASSERT(val.HasMember("type"), "State has no 'type' option");
        std::string stateType = val["type"].GetString();

        _stateNames.push_back(name);

        if (stateType == "Dominion")
        {
            int numDominionCards = val["DominionCards"].GetInt();
            _dominionStates[name] = { name, numDominionCards };
        }
        else
        {
            _stateMap[name] = GetStateFromVariable(name, rootValue);
        }
    }

    std::cout << "States Parsed: " << _stateMap.size() << "\n";
}

void AIParameters::parseBuyLimits(const std::string & keyName, const rapidjson::Value & rootValue)
{
    if (!rootValue.HasMember(keyName.c_str()))
    {
        return;
    }

    const rapidjson::Value & limits = rootValue[keyName.c_str()];
    for (rapidjson::Value::ConstMemberIterator itr = limits.MemberBegin(); itr != limits.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
        
        PRISMATA_ASSERT(val.IsArray(), "Filter must be an array");

        BuyLimits buyLimits(val);

        _buyLimits.insert(std::pair<std::string,BuyLimits>(name, buyLimits));
    }
}

void AIParameters::parseFilters(const std::string & keyName, const rapidjson::Value & rootValue)
{
    if (!rootValue.HasMember(keyName.c_str()))
    {
        return;
    }
    
    const rapidjson::Value & filters = rootValue[keyName.c_str()];
    for (rapidjson::Value::ConstMemberIterator itr = filters.MemberBegin(); itr != filters.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
        
        PRISMATA_ASSERT(val.IsObject(), "Filter must be an object");

        parseCardFilter(name, rootValue);
    }
}

CardFilter AIParameters::parseCardFilter(const std::string & filterVariable, const rapidjson::Value & root)
{
    if (_cardFilters.find(filterVariable) != _cardFilters.end())
    {
        return _cardFilters[filterVariable];
    }

    const rapidjson::Value & filterValue = findCardFilter(filterVariable.c_str(), root);

    CardFilter filter(filterVariable, filterValue);

    if (filterValue.HasMember("include"))
    {
        PRISMATA_ASSERT(filterValue["include"].IsString(), "Filter include must be a string");

        filter.addFilter(parseCardFilter(filterValue["include"].GetString(), root));
    }

    _cardFilters[filterVariable] = filter;

    return filter;
}

void AIParameters::parseOpeningBooks(const std::string & keyName, const rapidjson::Value & rootValue)
{
    if (!rootValue.HasMember(keyName.c_str()))
    {
        return;
    }

    const rapidjson::Value & openingBooks = rootValue[keyName.c_str()];
    for (rapidjson::Value::ConstMemberIterator itr = openingBooks.MemberBegin(); itr != openingBooks.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
        
        PRISMATA_ASSERT(val.IsArray(), "Opening book value must be an array");

        OpeningBook book;

        for (size_t i(0); i < val.Size(); ++i)
        {
            OpeningBookEntry entry(val[i]);

            // check to see if the entry is valid, since it may contain cards which are not in the current set
            if (entry.isValid())
            {
                book.push_back(entry);
            }
        }

        _openingBookMap[Players::Player_One][name] = book;
        _openingBookMap[Players::Player_Two][name] = book;
    }
}

void AIParameters::parsePartialPlayers(const std::string & keyName, const rapidjson::Value & rootValue)
{
    if (!rootValue.HasMember(keyName.c_str()))
    {
        return;
    }

    const rapidjson::Value & partialPlayers = rootValue[keyName.c_str()];
    for (rapidjson::Value::ConstMemberIterator itr = partialPlayers.MemberBegin(); itr != partialPlayers.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
        
        PRISMATA_ASSERT(val.IsObject(), "Partial Player value must be an Object");

        // we may have parsed one partial player withint he parsing of another, so if we've already done it, skip it
        if (_partialPlayerMap[Players::Player_One].find(name) == _partialPlayerMap[Players::Player_One].end())
        {
            parsePartialPlayer(Players::Player_One, name, rootValue);
            parsePartialPlayer(Players::Player_Two, name, rootValue);
        }

        _partialPlayerNames.push_back(name);
    }

    std::sort(_partialPlayerNames.begin(), _partialPlayerNames.end());
}

void AIParameters::parseMoveIterators(const std::string & keyName, const rapidjson::Value & rootValue)
{
    if (!rootValue.HasMember(keyName.c_str()))
    {
        return;
    }

    const rapidjson::Value & moveIterators = rootValue[keyName.c_str()];
    for (rapidjson::Value::ConstMemberIterator itr = moveIterators.MemberBegin(); itr != moveIterators.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
        
        PRISMATA_ASSERT(val.IsObject(), "Move Iterators value must be an Object");

        if (_moveIteratorMap[Players::Player_One].find(name) == _moveIteratorMap[Players::Player_One].end())
        {
            parseMoveIterator(Players::Player_One, name, rootValue);
            parseMoveIterator(Players::Player_Two, name, rootValue);

            //std::cout << "Move Iterator: " << name << "\n";
            //_moveIteratorMap[Players::Player_One][name]->print();
        }
    }
}

void AIParameters::parsePlayers(const std::string & keyName, const rapidjson::Value & rootValue)
{
    if (!rootValue.HasMember(keyName.c_str()))
    {
        return;
    }

    const rapidjson::Value & players = rootValue[keyName.c_str()];
    for (rapidjson::Value::ConstMemberIterator itr = players.MemberBegin(); itr != players.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
        
        PRISMATA_ASSERT(val.IsObject(), "Player value must be an Object");

        if (_moveIteratorMap[Players::Player_One].find(name) == _moveIteratorMap[Players::Player_One].end())
        {
            parsePlayer(Players::Player_One, name, rootValue);
            parsePlayer(Players::Player_Two, name, rootValue);
        }

        _playerNames.push_back(name);
    }

    std::sort(_playerNames.begin(), _playerNames.end());
}

PPPtr AIParameters::parsePartialPlayer(const PlayerID & player, const std::string & playerVariable, const rapidjson::Value & root)
{
    // if the partial player has already been parsed, no need to re-parse it, just return the already parsed version
    if (_partialPlayerMap[player].find(playerVariable) != _partialPlayerMap[player].end())
    {
        return getPartialPlayer(player, playerVariable);
    }

    const rapidjson::Value & playerValue = findPartialPlayer(playerVariable.c_str(), root);

    PRISMATA_ASSERT(playerValue.IsObject(), "Partial Player value is not an object");
    PRISMATA_ASSERT(playerValue.HasMember("type"), "PartialPlayer value has no 'type' option");

    std::string partialPlayerClassName = playerValue["type"].GetString();

    CardFilter filter;
    if (playerValue.HasMember("filter"))
    {
        PRISMATA_ASSERT(playerValue["filter"].IsString(), "Filter must be a string");
        const std::string filterName = playerValue["filter"].GetString();
        PRISMATA_ASSERT(_cardFilters.find(filterName) != _cardFilters.end(), "Filter string does not exist: %s", filterName.c_str());
        
        filter = _cardFilters[filterName];
    }

    PPPtr playerPtr;

    if (partialPlayerClassName == "Defense_Default")  
    { 
        playerPtr = PPPtr(new PartialPlayer_Defense_Default(player));
    }
    else if (partialPlayerClassName == "Defense_Solver")  
    { 
        playerPtr = PPPtr(new PartialPlayer_Defense_Solver(player));
    }
    else if (partialPlayerClassName == "Defense_GreedyKnapsack")  
    { 
        playerPtr = PPPtr(new PartialPlayer_Defense_GreedyKnapsack(player));
    }
    else if (partialPlayerClassName == "Defense_Random")  
    { 
        playerPtr = PPPtr(new PartialPlayer_Defense_Random(player));
    }
    else if (partialPlayerClassName == "ActionAbility_Default")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_Default(player));
    }
    else if (partialPlayerClassName == "ActionAbility_ActivateAll")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_ActivateAll(player));
    }
    else if (partialPlayerClassName == "ActionAbility_ActivateUtility")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_ActivateUtility(player, filter));
    }
    else if (partialPlayerClassName == "ActionAbility_AttackDefault")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_AttackDefault(player, filter));
    }
    else if (partialPlayerClassName == "ActionAbility_UntapAvoidBreach")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_UntapAvoidBreach(player));
    }
    else if (partialPlayerClassName == "ActionAbility_AvoidAttackWaste")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_AvoidAttackWaste(player));
    }
    else if (partialPlayerClassName == "ActionAbility_AvoidEconomyWaste")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_AvoidEconomyWaste(player));
    }
    else if (partialPlayerClassName == "ActionAbility_AvoidBreachSolver")  
    { 
        BreachIteratorParameters params;

        if (playerValue.HasMember("maxUntapDrones") && playerValue["maxUntapDrones"].IsInt())
        {
            params.maxUntapDrones = playerValue["maxUntapDrones"].GetInt();
        }

        if (playerValue.HasMember("chillCalculationMethod") && playerValue["chillCalculationMethod"].IsString())
        {
            const std::string & method = playerValue["chillCalculationMethod"].GetString();

            if (method == "None")
            {
                params.chillCalculationMethod = ChillCalculationMethod::NONE;
            }
            else if (method == "Solver")
            {
                params.chillCalculationMethod = ChillCalculationMethod::SOLVER;
            }
            else if (method == "Heuristic")
            {
                params.chillCalculationMethod = ChillCalculationMethod::HEURISTIC;
            }
        }

        if (playerValue.HasMember("maxChillSolverIterations") && playerValue["maxChillSolverIterations"].IsInt())
        {
            params.maxChillSolverIterations = playerValue["maxChillSolverIterations"].GetInt();
        }

        playerPtr = PPPtr(new PartialPlayer_ActionAbility_AvoidBreachSolver(player, params));
    }
    else if (partialPlayerClassName == "ActionAbility_EconomyDefault")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_EconomyDefault(player));
    }
    else if (partialPlayerClassName == "ActionAbility_FrontlineGreedyKnapsack")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_FrontlineGreedyKnapsack(player));
    }
    else if (partialPlayerClassName == "ActionAbility_ChillGreedyKnapsack")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_ChillGreedyKnapsack(player));
    }
    else if (partialPlayerClassName == "ActionAbility_ChillSolver")  
    { 
        size_t maxIterations = 0;

        if (playerValue.HasMember("maxIterations") && playerValue["maxIterations"].IsInt())
        {
            maxIterations = playerValue["maxIterations"].GetInt();
        }

        playerPtr = PPPtr(new PartialPlayer_ActionAbility_ChillSolver(player, maxIterations));
    }
    else if (partialPlayerClassName == "ActionAbility_SnipeGreedyKnapsack")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_SnipeGreedyKnapsack(player));
    }
    else if (partialPlayerClassName == "ActionAbility_Random")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_Random(player));
    }
    else if (partialPlayerClassName == "ActionAbility_Combination")  
    { 
        PRISMATA_ASSERT(playerValue.HasMember("combination"), "Combination args not found");
        PRISMATA_ASSERT(playerValue["combination"].IsArray(), "Combination isn't an array");
        
        std::vector<PPPtr> combo;
        for (size_t i(0); i < playerValue["combination"].Size(); ++i)
        {
            PPPtr comboPlayer = parsePartialPlayer(player, playerValue["combination"][i].GetString(), root);
            combo.push_back(comboPlayer);
        }

        playerPtr = PPPtr(new PartialPlayer_ActionAbility_Combination(player, combo));
    }
    else if (partialPlayerClassName == "ActionBuy_Default")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionBuy_Default(player));
    }
    else if (partialPlayerClassName == "ActionBuy_Nothing")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionBuy_Nothing(player));
    }
    else if (partialPlayerClassName == "ActionBuy_EngineerHeuristic")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionBuy_EngineerHeuristic(player));
    }
    else if (partialPlayerClassName == "ActionBuy_TechHeuristic")  
    { 
        size_t heuristicType = TechHeuristics::ELYOT_FORMULA;

        if (playerValue.HasMember("heuristic") && playerValue["heuristic"].IsString())
        {
            const std::string & heuristic = playerValue["heuristic"].GetString();

            if (heuristic == "ElyotFormula")
            {
                heuristicType = TechHeuristics::ELYOT_FORMULA;
            }
            else if (heuristic == "ElyotFormulaPlayout")
            {
                heuristicType = TechHeuristics::ELYOT_FORMULA_PLAYOUT;
            }
            else if (heuristic == "ElyotFormulaBalanced")
            {
                heuristicType = TechHeuristics::ELYOT_FORMULA_BALANCED;
            }
            else if (heuristic == "Diversify")
            {
                heuristicType = TechHeuristics::DIVERSIFY;
            }
            else
            {
                PRISMATA_ASSERT(false, "Unknown TechHeuristic type: %s", heuristic.c_str());
            }
        }

        playerPtr = PPPtr(new PartialPlayer_ActionBuy_TechHeuristic(player, heuristicType));
    }
    else if (partialPlayerClassName == "ActionBuy_Random")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionBuy_Random(player));
    }
    else if (partialPlayerClassName == "ActionBuy_OpeningBook")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionBuy_OpeningBook(player, _openingBookMap[player][playerValue["openingBook"].GetString()]));
    }
    else if (partialPlayerClassName == "ActionBuy_Combination")  
    { 
        PRISMATA_ASSERT(playerValue.HasMember("combination"), "Combination args not found");
        PRISMATA_ASSERT(playerValue["combination"].IsArray(), "Combination isn't an array");
        
        std::vector<PPPtr> combo;
        for (size_t i(0); i < playerValue["combination"].Size(); ++i)
        {
            PPPtr comboPlayer = parsePartialPlayer(player, playerValue["combination"][i].GetString(), root);
            combo.push_back(comboPlayer);
        }

        playerPtr = PPPtr(new PartialPlayer_ActionBuy_Combination(player, combo));
    }
    else if (partialPlayerClassName == "ActionBuy_GreedyKnapsack")  
    { 
        if (playerValue.HasMember("heuristic") && playerValue["heuristic"].IsString())
        {
            const std::string & heuristic = playerValue["heuristic"].GetString();
            
            if (heuristic == "BuyWillScore")
            {
                playerPtr = PPPtr(new PartialPlayer_ActionBuy_GreedyKnapsack(player, filter, &Heuristics::BuyHighestCost));
            }
            else if (heuristic == "BuyAttackValue")
            {
                playerPtr = PPPtr(new PartialPlayer_ActionBuy_GreedyKnapsack(player, filter, &Heuristics::BuyAttackValue));
            }
            else if (heuristic == "BuyBlockValue")
            {
                playerPtr = PPPtr(new PartialPlayer_ActionBuy_GreedyKnapsack(player, filter, &Heuristics::BuyBlockValue));
            }
            else
            {
                PRISMATA_ASSERT(false, "Unknown heuristic type for ActionBuy_GreedyKnapsack: %s", heuristic.c_str());
            }
        }
    }
    else if (partialPlayerClassName == "ActionBuy_Sequence")  
    {
        PRISMATA_ASSERT(playerValue.HasMember("buySequence") && playerValue["buySequence"].IsArray(), "ActionBuy_Sequence player must have a buySequence member array");
        const rapidjson::Value & buySequenceVal = playerValue["buySequence"];

        BuySequence sequence;

        for (size_t i(0); i < buySequenceVal.Size(); ++i)
        {
            PRISMATA_ASSERT(buySequenceVal[i].IsArray() && buySequenceVal[i].Size() == 2, "buySequence array member must be an array of size 2");
            PRISMATA_ASSERT(buySequenceVal[i][0u].IsString() && buySequenceVal[i][1u].IsInt(), "buySequence array member must have form [\"CardName\", LimitInt]");

            const std::string & cardName = buySequenceVal[i][0u].GetString();
            const int buyMax = buySequenceVal[i][1u].GetInt();

            if (CardTypes::CardTypeExists(cardName))
            {
                sequence.push_back(std::pair<CardType, size_t>(CardTypes::GetCardType(cardName), buyMax));
            }
        }

        playerPtr = PPPtr(new PartialPlayer_ActionBuy_Sequence(player, sequence));
    }
    else if (partialPlayerClassName == "Breach_Default")  
    { 
        playerPtr = PPPtr(new PartialPlayer_Breach_Default(player));
    }
    else if (partialPlayerClassName == "Breach_Random")  
    { 
        playerPtr = PPPtr(new PartialPlayer_Breach_Random(player));
    }
    else if (partialPlayerClassName == "Breach_GreedyKnapsack")  
    { 
        bool lowTechPriority = true;
        if (playerValue.HasMember("lowTechPriority"))
        {
            PRISMATA_ASSERT(playerValue["lowTechPriority"].IsBool(), "BreachGreedyKnapsack's 'lowTechPriority' must be a bool");

            lowTechPriority = playerValue["lowTechPriority"].GetBool();
        }

        playerPtr = PPPtr(new PartialPlayer_Breach_GreedyKnapsack(player, lowTechPriority));
    }
    else if (partialPlayerClassName == "ActionAbility_DontAttack")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionAbility_DontAttack(player));
    }
    else if (partialPlayerClassName == "ActionBuy_AvoidBreach")  
    { 
        playerPtr = PPPtr(new PartialPlayer_ActionBuy_AvoidBreach(player));
    }
    else
    {
        std::cout << "WARNING: Unknown Partial Player Class Name:" << partialPlayerClassName << "\n";
    }
    
    _partialPlayerParses++;
    playerPtr->setDescription("   " + playerVariable);

    BuyLimits buyLimits;
    if (playerValue.HasMember("buyLimits"))
    {
        PRISMATA_ASSERT(playerValue["buyLimits"].IsString(), "BuyLimits must be a string");
        const std::string buyLimitName = playerValue["buyLimits"].GetString();
        PRISMATA_ASSERT(_buyLimits.find(buyLimitName) != _buyLimits.end(), "BuyLimits string does not exist: %s", buyLimitName.c_str());

        buyLimits = _buyLimits[buyLimitName];
        playerPtr->setBuyLimits(buyLimits);
    }
    
    _partialPlayerMap[player][playerVariable] = playerPtr;
    
    return playerPtr->clone();
}

PlayerPtr AIParameters::parsePlayer(const PlayerID & player, const std::string & playerVariable, const rapidjson::Value & root)
{
    // if the player has already been parsed, no need to re-parse it, just return the already parsed version
    if (_playerMap[player].find(playerVariable) != _playerMap[player].end())
    {
        return getPlayer(player, playerVariable);
    }

    const rapidjson::Value & playerValue = findPlayer(playerVariable.c_str(), root);

    PRISMATA_ASSERT(playerValue.HasMember("type"), "Player has no 'type' option");

    std::string playerClassName = playerValue["type"].GetString();
    
    PlayerPtr playerPtr;

    if (playerClassName == "Player_Default")  
    { 
        playerPtr = PlayerPtr(new Player_Default(player));
    }
    else if (playerClassName == "Player_Random")  
    { 
        playerPtr = PlayerPtr(new Player_Random(player));
    }
    else if (playerClassName == "Player_RandomFromIterator")  
    { 
        PRISMATA_ASSERT(playerValue.HasMember("iterator"), "Player_RandomFromIterator has no iterator value");

        playerPtr = PlayerPtr(new Player_RandomFromIterator(player, getMoveIterator(player, playerValue["iterator"].GetString())));
    }
    else if (playerClassName == "Player_GUI")  
    {
        playerPtr = PlayerPtr(new Player_GUI(player));
    }
    else if (playerClassName == "Player_PPSequence")  
    { 
        PRISMATA_ASSERT(playerValue.HasMember("PartialPlayers"), "Partial players args not found");

        PPSequence ppSequence;

        for (size_t pp(0); pp<playerValue["PartialPlayers"].Size(); ++pp)
        {
            PRISMATA_ASSERT(playerValue["PartialPlayers"][pp].IsString(), "PPSequence player entry must be a string referring to a partial player variable");

            PPPtr ppptr = getPartialPlayer(player, playerValue["PartialPlayers"][pp].GetString());
            ppSequence[pp] = ppptr;
        }
        
        playerPtr = PlayerPtr(new Player_PPSequence(player, ppSequence));
    }
    else if (playerClassName == "Player_UCT")  
    { 
        const rapidjson::Value & args = playerValue;

        PRISMATA_ASSERT(args.HasMember("TimeLimit"), "No UCT TimeLimit Found");
        PRISMATA_ASSERT(args.HasMember("MaxTraversals"), "No UCT max traversals Found");
        PRISMATA_ASSERT(args.HasMember("MaxChildren"), "No UCT MaxChildren Found");
        PRISMATA_ASSERT(args.HasMember("MoveIterator"), "No UCT MoveIterator Found");
        PRISMATA_ASSERT(args.HasMember("Eval"), "No UCT EvalType Found");
        
        UCTSearchParameters params;
        params.setMaxPlayer(player);
        params.setTimeLimit(args["TimeLimit"].GetInt());
        params.setMaxTraversals(args["MaxTraversals"].GetInt());
        params.setMaxChildren(args["MaxChildren"].GetInt());
        params.setMoveIterator(Players::Player_One, getMoveIterator(Players::Player_One, args["MoveIterator"].GetString()));
        params.setMoveIterator(Players::Player_Two, getMoveIterator(Players::Player_Two, args["MoveIterator"].GetString()));

        if (args.HasMember("RootMoveIterator"))
        {
            params.setRootMoveIterator(Players::Player_One, getMoveIterator(Players::Player_One, args["RootMoveIterator"].GetString()));
            params.setRootMoveIterator(Players::Player_Two, getMoveIterator(Players::Player_Two, args["RootMoveIterator"].GetString()));
        }
        else
        {
            params.setRootMoveIterator(Players::Player_One, getMoveIterator(Players::Player_One, args["MoveIterator"].GetString()));
            params.setRootMoveIterator(Players::Player_Two, getMoveIterator(Players::Player_Two, args["MoveIterator"].GetString()));
        }

        std::string evalMethodString = args["Eval"].GetString();
        if (evalMethodString == "Playout")
        {
            params.setEvalMethod(EvaluationMethods::Playout);

            PRISMATA_ASSERT(args.HasMember("PlayoutPlayer"), "No playout player found");

            params.setPlayoutPlayer(Players::Player_One, parsePlayer(Players::Player_One, args["PlayoutPlayer"].GetString(), root));
            params.setPlayoutPlayer(Players::Player_Two, parsePlayer(Players::Player_Two, args["PlayoutPlayer"].GetString(), root));
        }
        else if (evalMethodString == "WillScore")
        {
            params.setEvalMethod(EvaluationMethods::WillScore);
        }
        else if (evalMethodString == "WillScoreInflation")
        {
            params.setEvalMethod(EvaluationMethods::WillScoreInflation);
        }
        else
        {
            PRISMATA_ASSERT(false, "Unknown UCT Evaluation Method Name: %s", evalMethodString.c_str());
        }
        
        if (args.HasMember("UCTConstant") && args["UCTConstant"].IsDouble())
        {
            params.setCValue(args["UCTConstant"].GetDouble());
        }

        //params.setGraphVizFilename("uct.png");
        
        playerPtr = PlayerPtr(new Player_UCT(player, params));
    }
    else if (playerClassName == "Player_StackAlphaBeta" || playerClassName == "Player_AlphaBeta")  
    { 
        const rapidjson::Value & args = playerValue;

        PRISMATA_ASSERT(args.HasMember("TimeLimit"), "No SAB TimeLimit Found");
        PRISMATA_ASSERT(args.HasMember("MaxChildren"), "No Max Children Found");
        PRISMATA_ASSERT(args.HasMember("MoveIterator"), "No MoveIterator Found");
        PRISMATA_ASSERT(args.HasMember("Eval"), "No eval type Found");
        
        AlphaBetaSearchParameters params;
        params.setMaxPlayer(player);
        params.setTimeLimit(args["TimeLimit"].GetInt());
        params.setMaxChildren(args["MaxChildren"].GetInt());
        params.setMoveIterator(Players::Player_One, getMoveIterator(Players::Player_One, args["MoveIterator"].GetString()));
        params.setMoveIterator(Players::Player_Two, getMoveIterator(Players::Player_Two, args["MoveIterator"].GetString()));

        if (args.HasMember("RootMoveIterator"))
        {
            params.setRootMoveIterator(Players::Player_One, getMoveIterator(Players::Player_One, args["RootMoveIterator"].GetString()));
            params.setRootMoveIterator(Players::Player_Two, getMoveIterator(Players::Player_Two, args["RootMoveIterator"].GetString()));
        }
        else
        {
            params.setRootMoveIterator(Players::Player_One, getMoveIterator(Players::Player_One, args["MoveIterator"].GetString()));
            params.setRootMoveIterator(Players::Player_Two, getMoveIterator(Players::Player_Two, args["MoveIterator"].GetString()));
        }

        if (args.HasMember("MaxDepth") && args["MaxDepth"].IsInt())
        {
            params.setMaxDepth(args["MaxDepth"].GetInt());
        }

        std::string evalMethodString = args["Eval"].GetString();
        if (evalMethodString == "Playout")
        {
            params.setEvalMethod(EvaluationMethods::Playout);

            PRISMATA_ASSERT(args.HasMember("PlayoutPlayer"), "No playout player found");

            params.setPlayoutPlayer(Players::Player_One, parsePlayer(Players::Player_One, args["PlayoutPlayer"].GetString(), root));
            params.setPlayoutPlayer(Players::Player_Two, parsePlayer(Players::Player_Two, args["PlayoutPlayer"].GetString(), root));
        }
        else if (evalMethodString == "WillScore")
        {
            params.setEvalMethod(EvaluationMethods::WillScore);
        }
        else if (evalMethodString == "WillScoreInflation")
        {
            params.setEvalMethod(EvaluationMethods::WillScoreInflation);
        }
        else
        {
            PRISMATA_ASSERT(false, "Unknown SAB Evaluation Method Name: %s", evalMethodString.c_str());
        }

        if (playerClassName == "Player_AlphaBeta")
        {
            playerPtr = PlayerPtr(new Player_AlphaBeta(player, params));
        }
        else if (playerClassName == "Player_StackAlphaBeta")
        {
            playerPtr = PlayerPtr(new Player_StackAlphaBeta(player, params));
        }
    }
    else
    {
        PRISMATA_ASSERT(false, "Unknown Player Class Name: %s", playerClassName.c_str());
    }

    _playerParses++;
    playerPtr->setDescription(playerVariable);

    _playerMap[player][playerVariable] = playerPtr;

    return playerPtr->clone();
}

MoveIteratorPtr AIParameters::parseMoveIterator(const PlayerID & player, const std::string & iteratorVariable, const rapidjson::Value & root)
{
    // if the move iterator has already been parsed, no need to re-parse it, just return the already parsed version
    if (_moveIteratorMap[player].find(iteratorVariable) != _moveIteratorMap[player].end())
    {
        return getMoveIterator(player, iteratorVariable);
    }

    const rapidjson::Value & iteratorValue = findMoveIterator(iteratorVariable.c_str(), root);

    PRISMATA_ASSERT(iteratorValue.HasMember("type"), "Iterator has no 'type' option");

    std::string moveIteratorTypeName = iteratorValue["type"].GetString();

    BuyLimits buyLimits;
    if (iteratorValue.HasMember("buyLimits"))
    {
        PRISMATA_ASSERT(iteratorValue["buyLimits"].IsString(), "BuyLimits must be a string");
        const std::string buyLimitName = iteratorValue["buyLimits"].GetString();
        PRISMATA_ASSERT(_buyLimits.find(buyLimitName) != _buyLimits.end(), "BuyLimits string does not exist: %s", buyLimitName.c_str());

        buyLimits = _buyLimits[buyLimitName];
    }

    MoveIteratorPtr mip;
    if (moveIteratorTypeName == "PPPortfolio")  
    { 
        if (iteratorValue.HasMember("include"))
        {
            PRISMATA_ASSERT(iteratorValue["include"].IsString(), "included move iterator must be a string");

            mip = parseMoveIterator(player, iteratorValue["include"].GetString(), root);
        }
        else
        {
            mip = MoveIteratorPtr(new MoveIterator_PPPortfolio(player));
        }

        if (iteratorValue.HasMember("PartialPlayers"))
        {
            const rapidjson::Value & ppArray = iteratorValue["PartialPlayers"];
            PRISMATA_ASSERT(ppArray.Size() == PPPhases::NUM_PHASES, "PP array in move iterator must be same length as number of phases: %d", PPPhases::NUM_PHASES);

            for (size_t pp(0); pp<ppArray.Size(); ++pp)
            {
                PRISMATA_ASSERT(ppArray[pp].IsArray(), "MoveIterator PP entry must be an array");

                for (size_t i(0); i < ppArray[pp].Size(); ++i)
                {
                    PPPtr ppptr = getPartialPlayer(player, ppArray[pp][i].GetString());
                    ((MoveIterator_PPPortfolio *)mip.get())->addPartialPlayer(pp, ppptr);
                }
            }
        }
    }
    else
    {
        PRISMATA_ASSERT(false, "Unknown MoveIterator Class Name: %s", moveIteratorTypeName.c_str());
    }

    _moveIteratorParses++;

    if (buyLimits.hasAnyLimits())
    {
        mip->setBuyLimits(buyLimits);
    }

    _moveIteratorMap[player][iteratorVariable] = mip;

    return mip->clone();
}

PlayerPtr AIParameters::getPlayer(const PlayerID & player, const std::string & playerName)
{
    PRISMATA_ASSERT(_playerMap[player].find(playerName) != _playerMap[player].end(), "AIParameters::getPlayer Couldn't find player variable: %d %s", (int)_playerMap[player].size(), playerName.c_str());

    return _playerMap[player][playerName]->clone();
}

PPPtr AIParameters::getPartialPlayer(const PlayerID & player, const std::string & playerName)
{
	PRISMATA_ASSERT(_partialPlayerMap[player].find(playerName) != _partialPlayerMap[player].end(), "AIParameters::getPartialPlayer Couldn't find player variable  %d %s", (int)_partialPlayerMap[player].size(), playerName.c_str());

    return _partialPlayerMap[player][playerName]->clone();
}

MoveIteratorPtr AIParameters::getMoveIterator(const PlayerID & player, const std::string & iteratorName)
{
	PRISMATA_ASSERT(_moveIteratorMap[player].find(iteratorName) != _moveIteratorMap[player].end(), "AIParameters::getMoveIterator Couldn't find movw iterator variable %d %s", (int)_moveIteratorMap[player].size(), iteratorName.c_str());

    return _moveIteratorMap[player][iteratorName]->clone();
}

void AIParameters::parseJSONString(const std::string & jsonString)
{
    rapidjson::Document document;
    bool parsingFailed = document.Parse(jsonString.c_str()).HasParseError();

    if (parsingFailed)
    {
        int errorPos = document.GetErrorOffset();

        std::stringstream ss;
        ss << std::endl << "JSON Parse Error: " << document.GetParseError() << std::endl;
        ss << "Error Position:   " << errorPos << std::endl;
        ss << "Error Substring:  " << jsonString.substr(errorPos-5, 10) << std::endl;

        PRISMATA_ASSERT(!parsingFailed, "Error parsing JSON config file: %s", ss.str().c_str());
    }

    parseJSONValue(document);
}

void AIParameters::parseFile(const std::string & filename)
{
    std::cout << "Reading AI Parameters File: " << filename << "\n";
    std::string json = FileUtils::ReadFile(filename);
    
    parseJSONString(json);
}

const std::vector<std::string> & AIParameters::getPlayerNames() const
{
    return _playerNames;
}

const std::vector<std::string> & AIParameters::getPartialPlayerNames() const
{
    return _partialPlayerNames;
}

const rapidjson::Value & AIParameters::findPlayer(const std::string & playerName, const rapidjson::Value & rootValue)
{
    for (size_t i(0); i < _playerKeyNames.size(); ++i)
    {
        if (rootValue.HasMember(_playerKeyNames[i].c_str()))
        {
            if (rootValue[_playerKeyNames[i].c_str()].HasMember(playerName.c_str()))
            {
                return rootValue[_playerKeyNames[i].c_str()][playerName.c_str()];
            }
        }
    }

    PRISMATA_ASSERT(false, "Player not found: %s", playerName.c_str());
    return rootValue;
}

const rapidjson::Value & AIParameters::findPartialPlayer(const std::string & playerName, const rapidjson::Value & rootValue)
{
    for (size_t i(0); i < _partialPlayerKeyNames.size(); ++i)
    {
        if (rootValue.HasMember(_partialPlayerKeyNames[i].c_str()))
        {
            if (rootValue[_partialPlayerKeyNames[i].c_str()].HasMember(playerName.c_str()))
            {
                return rootValue[_partialPlayerKeyNames[i].c_str()][playerName.c_str()];
            }
        }
    }

    PRISMATA_ASSERT(false, "Player not found: %s", playerName.c_str());
    return rootValue;
}

const rapidjson::Value & AIParameters::findMoveIterator(const std::string & iteratorName, const rapidjson::Value & rootValue)
{
    for (size_t i(0); i < _moveIteratorKeyNames.size(); ++i)
    {
        if (rootValue.HasMember(_moveIteratorKeyNames[i].c_str()))
        {
            if (rootValue[_moveIteratorKeyNames[i].c_str()].HasMember(iteratorName.c_str()))
            {
                return rootValue[_moveIteratorKeyNames[i].c_str()][iteratorName.c_str()];
            }
        }
    }

    PRISMATA_ASSERT(false, "Player not found: %s", iteratorName.c_str());
    return rootValue;
}

const rapidjson::Value & AIParameters::findCardFilter(const std::string & filterName, const rapidjson::Value & rootValue)
{
    for (size_t i(0); i < _filterKeyNames.size(); ++i)
    {
        if (rootValue.HasMember(_filterKeyNames[i].c_str()))
        {
            if (rootValue[_filterKeyNames[i].c_str()].HasMember(filterName.c_str()))
            {
                return rootValue[_filterKeyNames[i].c_str()][filterName.c_str()];
            }
        }
    }

    PRISMATA_ASSERT(false, "CardFilter not found: %s", filterName.c_str());
    return rootValue;
}

GameState AIParameters::GetStateFromVariable(const std::string & stateVariable, const rapidjson::Value & root)
{
    PRISMATA_ASSERT(root["States"].HasMember(stateVariable.c_str()), "State variable not found");

    const rapidjson::Value & stateValue = root["States"][stateVariable.c_str()];

    PRISMATA_ASSERT(stateValue.HasMember("type"), "State has no 'type' option");

    std::string stateType = stateValue["type"].GetString();

    if (stateType.compare("JSON") == 0)
    {
        PRISMATA_ASSERT(stateValue.HasMember("filename"), "JSON State has no 'filename' option");

        std::string fileContents = FileUtils::ReadFile(stateValue["filename"].GetString());
        rapidjson::Document document;
        bool parsingFailed = document.Parse(fileContents.c_str()).HasParseError();
    
        PRISMATA_ASSERT(!parsingFailed, "Parsing of JSON GameState failed: %s", fileContents.c_str());

        return GameState(document);
    } 
    else if (stateType.compare("Dominion") == 0)
    {
        PRISMATA_ASSERT(stateValue.HasMember("DominionCards"), "Dominion State 'args' has no 'DominionCards' option");
        
        int numDominionCards = stateValue["DominionCards"].GetInt();
       
        GameState state;
        state.setStartingState(Players::Player_One, numDominionCards);
        
        return state;
    }
    else
    {
        PRISMATA_ASSERT(false, "Unknown state type: %s", stateType.c_str());
    }

    return GameState();
}

const std::vector<std::string> & AIParameters::getStateNames() const
{
    return _stateNames;
}

GameState AIParameters::getState(const std::string & stateName)
{
    if (_dominionStates.find(stateName) != _dominionStates.end())
    {
        GameState state;
        state.setStartingState(Players::Player_One, _dominionStates[stateName].RandomCards);
        return state;
    }
    else
    {
        return _stateMap[stateName];
    }
}