#include "GameState.h"
#include <set>

using namespace Prismata;

GameState::GameState()
{
    
}

GameState::GameState(const rapidjson::Value & value)
    : GameState()
{
    initFromJSON(value);
}

void GameState::initFromJSON(const rapidjson::Value & value)
{
    std::vector<Card> cardsToAdd;
    std::vector< std::vector<CardID> > totalSupply(2, std::vector<CardID>());
    std::vector< std::vector<CardID> > supplySpent(2, std::vector<CardID>());
    std::vector< std::string > buyableCardNames;

    for (rapidjson::Value::ConstMemberIterator itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr)
    {
        const std::string &         prop = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;
             
        if (prop == "phase")
        {
            PRISMATA_ASSERT(val.IsString(), "GameState JSON phase was not a String");
            const std::string & phase = val.GetString();

            if (phase == "action")   
            {
                m_activePhase = Phases::Action;
            }
            else if (phase == "defense")   
            {
                m_activePhase = Phases::Defense;
            }
            else if (phase == "confirm")   
            {
                m_activePhase = Phases::Confirm;
            }
            else
            {
                PRISMATA_ASSERT(false, "Unknown GameState JSON Phase: %s", phase.c_str());
            }
        }
        else if (prop == "turn")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON turn was not an Int");
            m_activePlayer = val.GetInt();
        }
        else if (prop == "numTurns")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON numTurns was not an Int");
            m_turnNumber = val.GetInt();
        }
        else if (prop == "whiteMana")
        {
            PRISMATA_ASSERT(val.IsString(), "GameState JSON whiteMana was not a String");
            m_resources[Players::Player_One] = Resources(std::string(val.GetString()));
        }
        else if (prop == "blackMana")
        {
            PRISMATA_ASSERT(val.IsString(), "GameState JSON blackMana was not a String");
            m_resources[Players::Player_Two] = Resources(std::string(val.GetString()));
        }
        else if (prop == "whiteTotalSupply")
        {
            PRISMATA_ASSERT(val.IsArray(), "GameState JSON whiteTotalSupply was not an Array");
            
            for (rapidjson::SizeType i = 0; i < val.Size(); i++) 
            {
                PRISMATA_ASSERT(val[i].IsInt(), "GameState JSON whiteTotalSupply[%d] was not an Int", (int)i);
                totalSupply[Players::Player_One].push_back(val[i].GetInt());
            }   
        }
        else if (prop == "blackTotalSupply")
        {
            PRISMATA_ASSERT(val.IsArray(), "GameState JSON blackTotalSupply was not an Array");
            
            for (rapidjson::SizeType i = 0; i < val.Size(); i++) 
            {
                PRISMATA_ASSERT(val[i].IsInt(), "GameState JSON blackTotalSupply[%d] was not an Int", (int)i);
                totalSupply[Players::Player_Two].push_back(val[i].GetInt());
            }   
        }
        else if (prop == "whiteSupplySpent")
        {
            PRISMATA_ASSERT(val.IsArray(), "GameState JSON whiteSupplySpent was not an Array");
            
            for (rapidjson::SizeType i = 0; i < val.Size(); i++) 
            {
                PRISMATA_ASSERT(val[i].IsInt(), "GameState JSON whiteSupplySpent[%d] was not an Int", (int)i);
                supplySpent[Players::Player_One].push_back(val[i].GetInt());
            } 
        }
        else if (prop == "blackSupplySpent")
        {
            PRISMATA_ASSERT(val.IsArray(), "GameState JSON blackSupplySpent was not an Array");
            
            for (rapidjson::SizeType i = 0; i < val.Size(); i++) 
            {
                PRISMATA_ASSERT(val[i].IsInt(), "GameState JSON blackSupplySpent[%d] was not an Int", (int)i);
                supplySpent[Players::Player_Two].push_back(val[i].GetInt());
            }
        }
        else if (prop == "cards")
        {
            PRISMATA_ASSERT(val.IsArray(), "GameState JSON cards was not an Array");
            
            for (rapidjson::SizeType i = 0; i < val.Size(); i++) 
            {
                PRISMATA_ASSERT(val[i].IsString(), "GameState JSON cards[%d] was not a String", (int)i);
                buyableCardNames.push_back(val[i].GetString());
            }
        }
        else if (prop == "table")
        {
            PRISMATA_ASSERT(val.IsArray(), "GameState JSON table was not an Array");
                        
            for (rapidjson::SizeType i = 0; i < val.Size(); i++) 
            {
                PRISMATA_ASSERT(val[i].IsObject(), "GameState JSON table[%d] was not an Object", (int)i);
                
                int amount = 1;
                if (val[i].HasMember("amount") && val[i]["amount"].IsInt())
                {
                    amount = val[i]["amount"].GetInt();
                }
                
                for (int c(0); c < amount; ++c)
                {
                    cardsToAdd.push_back(Card(val[i]));
                }
            }
        }
        else
        {

        }
    }

    // add the buyable cards
    if (!value.HasMember("whiteSupplySpent")) { supplySpent[Players::Player_One] = std::vector<CardID>(buyableCardNames.size(), 0); }
    if (!value.HasMember("blackSupplySpent")) { supplySpent[Players::Player_Two] = std::vector<CardID>(buyableCardNames.size(), 0); }

    if (!value.HasMember("whiteTotalSupply")) 
    { 
        for (const auto & typeName : buyableCardNames)
        {
            totalSupply[Players::Player_One].push_back(CardTypes::GetCardType(typeName).getSupply());
        }
    }
    
    if (!value.HasMember("blackTotalSupply")) 
    { 
        for (const auto & typeName : buyableCardNames)
        {
            totalSupply[Players::Player_Two].push_back(CardTypes::GetCardType(typeName).getSupply());
        }
    }

    PRISMATA_ASSERT(supplySpent[0].size() == buyableCardNames.size(), "Card names array not same length as supply array");
    for (size_t i(0); i < buyableCardNames.size(); ++i)
    {
        if (totalSupply[Players::Player_One][i] > 0 || totalSupply[Players::Player_Two][i] > 0)
        {
            CardBuyable cb(CardTypes::GetCardType(buyableCardNames[i]), totalSupply[Players::Player_One][i], totalSupply[Players::Player_Two][i], supplySpent[Players::Player_One][i], supplySpent[Players::Player_Two][i]);
            m_cards.addBuyableCard(cb);
        }
    }

    // add the cards
    for (const Card & card : cardsToAdd)
    {
        if (!card.isDead())
        {
            m_cards.addCard(card);
        }
    }
    
    beginPhase(m_activePlayer, m_activePhase);
}

bool GameState::isLegal(const Action & action) const
{
    if (action.getPlayer() != getActivePlayer())
    {
        return false;
    }

    const PlayerID player = action.getPlayer();
    const PlayerID enemy = getEnemy(player);

    switch(action.getType())
    {
        case ActionTypes::BUY:
        {
            if (getActivePhase() != Phases::Action)
            {
                return false;
            }

            if (!isBuyable(player, CardType(action.getID())))
            {
                return false;
            }

            if (!getResources(player).has(getCardBuyableByID(action.getID()).getType().getBuyCost()))
            {
                return false;
            }

            if (!haveSacCost(player, getCardBuyableByID(action.getID()).getType().getBuySac()))
            {
                return false;
            }

            if (getCardBuyableByID(action.getID()).getSupplyRemaining(player) == 0)
            {
                return false;
            }

            return true;
        }
        case ActionTypes::SELL:
        {
            const Card & card = getCardByID(action.getID());

            if (getActivePhase() != Phases::Action)
            {
                return false;
            }

            if (!card.isSellable())
            {
                return false;
            }

            if (!(card.getPlayer() == player))
            {
                return false;
            }

            if (!canRunScriptUndo(action.getPlayer(), action.getID(), card.getType().getBuyScript()))
            {
                return false;
            }

            return true;
        }
        case ActionTypes::SNIPE:
        case ActionTypes::CHILL:
        {
            const Card & card = getCardByID(action.getID());
            const Card & targetCard = getCardByID(action.getTargetID());

            if (!isTargetAbilityCardClicked())
            {
                return false;
            }

            // can't snipe a dead card
            if (targetCard.isDead())
            {
                return false;
            }

            // we can't target our own cards with target abilities
            if (player == getCardByID(action.getTargetID()).getPlayer())
            {
                return false;
            }
            
            // check to see if the snipe condition is met
            if ((action.getType() == ActionTypes::SNIPE) && !targetCard.meetsCondition(card.getType().getTargetAbilityCondition()))
            {
                return false;
            }

            if ((action.getType() == ActionTypes::CHILL) && !targetCard.canBeChilled())
            {
                return false;
            }
            
            return true;
        }
        case ActionTypes::USE_ABILITY:
        {
            const Card & card = getCardByID(action.getID());

            // must be action phase to issue an ability
            if (getActivePhase() != Phases::Action)
            {
                return false;
            }

            if (isTargetAbilityCardClicked())
            {
                return false;
            }

            if (card.isDead())
            {
                return false;
            }

            // check to see if the card has an ability
            if (!card.getType().hasAbility() && !card.getType().hasTargetAbility())
            {
                return false;
            }

            // check if the card is in the correct state to assign
            if (!card.canUseAbility())
            {
                return false;
            }
               
            // check to see if we can the script
            if (!canRunScript(player, getCardByID(action.getID()).getType().getAbilityScript()))
            {
                return false;
            }

            return true;
        }
        case ActionTypes::END_PHASE:
        {
            switch (getActivePhase())
            {
                case Phases::Defense:
                {
                    return getAttack(enemy) == 0;
                }
                case Phases::Swoosh:
                {
                    return false;
                }
                case Phases::Action:
                {
                    return true;
                }
                case Phases::Breach:
                {
                    if (numCards(enemy) == 0)
                    {
                        return true;
                    }

                    if (getAttack(player) == 0)
                    {
                        return true;
                    }

                    // we can end breach phase if they have a breachable card but we don't have enough damage to kill it
                    if (hasBreachableCard(enemy) && !canBreachEnemyCard(player))
                    {
                        return true;
                    }

                    if (hasOverkillableCard(enemy) && !canOverkillEnemyCard(player))
                    {
                        return true;
                    }

                    return false;
                }
                case Phases::Confirm:
                {
                    return true;
                }
            }
        }
        case ActionTypes::WIPEOUT:
        {
            if (!getCardByID(action.getID()).canBlock())
            {
                return false;
            }

            if (!canWipeout(action.getPlayer()))
            {
                return false;
            }

            return true;
        }
        case ActionTypes::UNDO_USE_ABILITY:
        {
            const Card & card = getCardByID(action.getID());

            if (getActivePhase() != Phases::Action)
            {
                return false;
            }

            if (isTargetAbilityCardClicked() && (action.getID() == m_targetAbilityCardID))
            {
                return true;
            }

            if (card.isDead() && (!card.isInPlay() || !card.selfKilled()))
            {
                return false;
            }

            // if the card isn't assigned then it wasn't used so we can't undo it
            if (card.getStatus() != CardStatus::Assigned)
            {
                return false;
            }
            
            // undo snipe is not implemented yet
            if (card.getType().hasTargetAbility() && card.getType().getTargetAbilityType() == ActionTypes::SNIPE)
            {
                return false;
            }

            if (!card.getType().hasAbility() && !card.getType().hasTargetAbility())
            {
                return false;
            }
            
            if (!canRunScriptUndo(action.getPlayer(), action.getID(), card.getType().getAbilityScript()))
            {
                return false;
            }

            return true;
        }
        case ActionTypes::UNDO_CHILL:
        {
            const Card & card = getCardByID(action.getID());

            if (card.isDead())
            {
                return false;
            }

            if (card.currentChill() == 0)
            {
                return false;
            }

            return true;
        }
        case ActionTypes::ASSIGN_BLOCKER:
        {
            return (getAttack(enemy) > 0) && (getActivePhase() == Phases::Defense) && getCardByID(action.getID()).canBlock();
        }
        case ActionTypes::ASSIGN_FRONTLINE:
        {
            const Card & target = getCardByID(action.getID());

            if (getActivePhase() != Phases::Action)
            {
                return false;
            }

            if (player == target.getPlayer())
            {
                return false;
            }

            return target.canFrontlineFor(getAttack(action.getPlayer()));
        }
        case ActionTypes::ASSIGN_BREACH:
        {
            const Card & target = getCardByID(action.getID());
            if (target.getPlayer() == getActivePlayer())
            {
                return false;
            }

            if (getAttack(action.getPlayer()) == 0)
            {
                return false;
            }

            if (getActivePhase() != Phases::Breach)
            {
                return false;
            }

            if (target.isOverkillable())
            {
                if (!canOverkillEnemyCard(action.getPlayer()))
                {
                    return false;
                }
                else
                {
                    return target.canOverkillFor(getAttack(getActivePlayer()));
                }
            }
            else
            {
                if (!target.canBreachFor(getAttack(getActivePlayer())))
                {
                    return false;
                }

                if (target.isFrozen() && !canBreachFrozenCard())
                {
                    return false;
                }
            }

            return true;
        }
        case ActionTypes::UNDO_BREACH:
        {
            const Card & card = getCardByID(action.getID());
            
            if (getActivePhase() != Phases::Breach)
            {
                return false;
            }

            if (!card.wasBreached())
            {
                return false;
            }

            return true;
        }
        default:
        {
            PRISMATA_ASSERT(false, "Unimplemented Action Type: %s", action.toString().c_str());
        }
    }

    return false;
}
    
// carries out a given action
// WARNING: MAKE SURE ACTIONS ARE LEGAL IF checkLegal = false, UNDEFINED BEHAVIOUR OTHERWISE
bool GameState::doAction(const Action & action)
{
    if (action == Action())
    {
        return false;
    }

    if (!isLegal(action))
    {
        bool legal = isLegal(action);
    }
    
    PRISMATA_ASSERT(isLegal(action), "Trying to do an illegal action! %s", action.toClientString().c_str());
    
    switch(action.getType())
    {
        case ActionTypes::BUY:
        {
            do
            {
                CardID cardID = buyCardByID(action.getPlayer(), action.getID()).getID();
                runScript(cardID, getCardByID(cardID).getType().getBuyScript(), ScriptTypes::BuyScript);
            }
            while (action.getShift() && isLegal(action));
                        
            break;
        }
        case ActionTypes::USE_ABILITY:
        {
            PRISMATA_ASSERT(isLegal(action), "Tried to do illegal Ability action, Card=%s", getCardByID(action.getID()).toJSONString().c_str());
            PRISMATA_ASSERT(!getCardByID(action.getID()).isDead(), "Tried to use dead card ability, Card=%s", getCardByID(action.getID()).toJSONString().c_str());
                        
            Action currentAction(action);
            const Card & card = getCardByID(currentAction.getID());
            bool legalShiftActionFound = false;
            size_t lastShiftCardChecked = 0;

            // if it's a target ability card we don't apply the effects of clicking yet
            if (card.getType().hasTargetAbility())
            {
                m_targetAbilityCardID = action.getID();
                m_targetAbilityCardClicked = true;
                return true;
            }

            do
            {
                legalShiftActionFound = false;

                // run the card's ability script
                runScript(currentAction.getID(), getCardByID(currentAction.getID()).getType().getAbilityScript(), ScriptTypes::AbilityScript);

                // if the action is shifted, look for another unit of this type to activate
                if (currentAction.getShift())
                {
                    // if the card was killed by clicking it we need to reset the iterator for validation
                    if (getCardByID(currentAction.getID()).isDead())
                    {
                        lastShiftCardChecked = 0;
                    }

                    while (lastShiftCardChecked < numCards(currentAction.getPlayer()))
                    {
                        const Card & newCard = getCardByID(getCardIDs(currentAction.getPlayer())[lastShiftCardChecked]);
                        if (newCard.getType() != card.getType())
                        {
                            lastShiftCardChecked++;
                            continue;
                        }

                        currentAction.setID(newCard.getID());
                        legalShiftActionFound = (newCard.getType().isEconCard() && newCard.canUseAbility()) || isLegal(currentAction);
                        if (legalShiftActionFound)
                        {
                            break;
                        }

                        lastShiftCardChecked++;
                    }
                }
            } while (legalShiftActionFound);

            break;
        }
        case ActionTypes::END_PHASE:
        {
            endPhase();
            break;
        }
        case ActionTypes::ASSIGN_BLOCKER:
        {
            PRISMATA_ASSERT(isLegal(action), "Tried to do illegal block action, Card=%s", getCardByID(action.getID()).toJSONString().c_str());

            blockWithCard(_getCardByID(action.getID()));

            break;
        }
        case ActionTypes::ASSIGN_BREACH:
        {
            PRISMATA_ASSERT(!getCardByID(action.getID()).isDead(), "Trying to breach dead card");
            breachCard(_getCardByID(action.getID()));

            if (!_getCardByID(action.getID()).getType().isFrontline())
            {
                m_canBreachFrozenCard = true;
            }
            break;
        }
        case ActionTypes::UNDO_BREACH:
        {
            undoBreachCard(_getCardByID(action.getID()));
            break;
        }
        case ActionTypes::ASSIGN_FRONTLINE:
        {
            PRISMATA_ASSERT(!getCardByID(action.getID()).isDead(), "Trying to frontline dead card");
            blockWithCard(_getCardByID(action.getID()));
            break;
        }
        case ActionTypes::SNIPE:
        {
            Card & card = _getCardByID(action.getID());
            Card & target = _getCardByID(action.getTargetID());

            PRISMATA_ASSERT(!card.isDead(), "Trying to CHILL with dead card");
            PRISMATA_ASSERT(!target.isDead(), "Trying to snipe a dead target card");
            
            card.setTargetID(target.getID());
            killCardByID(action.getTargetID(), CauseOfDeath::Sniped);
            
            runScript(action.getID(), card.getType().getAbilityScript(), ScriptTypes::AbilityScript);

            m_targetAbilityCardClicked = false;
            m_targetAbilityCardID = 0;

            break;
        }
        case ActionTypes::CHILL:
        {
            Card & card = _getCardByID(action.getID());
            Card & target = _getCardByID(action.getTargetID());

            PRISMATA_ASSERT(!card.isDead(), "Trying to CHILL with dead card");
            PRISMATA_ASSERT(!target.isDead(), "Trying to CHILL a dead target card");

            target.applyChill(card.getType().getTargetAbilityAmount());
            card.setTargetID(target.getID());
            
            runScript(action.getID(), card.getType().getAbilityScript(), ScriptTypes::AbilityScript);

            m_targetAbilityCardClicked = false;
            m_targetAbilityCardID = 0;

            break;
        }
        case ActionTypes::WIPEOUT:
        {
            endPhase();
        }
        case ActionTypes::UNDO_CHILL:
        {
            // if we are undoing a chill from breach phase we revert to action phase
            if (getActivePhase() == Phases::Breach)
            {
                beginPhase(getActivePlayer(), Phases::Action);
            }
            
            // find every card chilling this card and undo its ability
            std::vector<CardID> cardsToUndo;
            for (const auto & cardID : getCardIDs(action.getPlayer()))
            {
                Card & card = _getCardByID(cardID);

                if (card.hasTarget() && (card.getTargetID() == action.getID()))
                {
                    cardsToUndo.push_back(cardID);
                }
            }

            for (const auto & cardID : getKilledCardIDs(action.getPlayer()))
            {
                Card & card = _getCardByID(cardID);

                if (card.hasTarget() && (card.getTargetID() == action.getID()))
                {
                    cardsToUndo.push_back(cardID);
                }
            }

            for (const auto & cardID : cardsToUndo)
            {
                Action undoChiller(action.getPlayer(), ActionTypes::UNDO_USE_ABILITY, cardID);
                doAction(undoChiller);
            }

            break;
        }
        case ActionTypes::UNDO_USE_ABILITY:
        {
            Action currentAction(action);

            // if we're waiting for targeting info, just stop waiting for it
            if (isTargetAbilityCardClicked() && (action.getID() == m_targetAbilityCardID))
            {
                m_targetAbilityCardClicked = false;
                m_targetAbilityCardID = 0;
                return true;
            }

            bool legalShiftActionFound = false;

            do
            {
                legalShiftActionFound = false;
                Card & card = _getCardByID(currentAction.getID());

                PRISMATA_ASSERT(card.getStatus() == CardStatus::Assigned, "Trying to undo ability of unassigned card");
            
                runScriptUndo(currentAction.getID(), card.getType().getAbilityScript(), ScriptTypes::AbilityScript);
                
                // if the action is shifted, look for another unit of this type to activate
                if (currentAction.getShift())
                {
                    for (const auto & cardID : getCardIDs(currentAction.getPlayer()))
                    //for (size_t c(0); c < numCards(currentAction.getPlayer()); ++c)
                    {
                        const Card & newCard = getCardByID(cardID);
                        if (newCard.getType() != card.getType())
                        {
                            continue;
                        }

                        currentAction.setID(newCard.getID());
                        if (isLegal(currentAction))
                        {
                            legalShiftActionFound = true;
                            break;
                        }
                    }
                }
            }
            while (currentAction.getShift() && legalShiftActionFound);
            
            break;
        }
        case ActionTypes::SELL:
        {
            sellCardByID(action.getPlayer(), action.getID());

            break;
        }
        default:
        {
            PRISMATA_ASSERT(false, "Unimplemented Action Type: %s", action.toString().c_str());
            return false;
        }
    }

    return true;
}

bool GameState::undoTargetAbility(Card & card)
{
    // undo the target ability specific effects
    if (card.getType().hasTargetAbility())
    {
        PRISMATA_ASSERT(card.hasTarget(), "Undoing a target ability should have a target");

        Card & target = _getCardByID(card.getTargetID());

        switch (card.getType().getTargetAbilityType())
        {
            case ActionTypes::CHILL:
            {
                target.removeChill(card.getType().getTargetAbilityAmount());
                
                break;
            }
            default:
            {
                PRISMATA_ASSERT(false, "Trying to undo an unimplmented target ability type");
                return false;
            }
        }

        // this card no longer has a target
        card.clearTarget();
    }

    return true;
}

bool GameState::canRunScript(const PlayerID player, const Script & script) const
{
    // CHECK IF WE HAVE MANA COST
    if (script.hasManaCost() && !getResources(player).has(script.getManaCost()))
    {
        return false;
    }

    // CHECK IF WE HAVE SAC COST
    if (script.hasSacCost() && !haveSacCost(player, script.getSacCost()))
    {
        return false;
    }

    // CHECK IF WE HAVE THE UNITS TO DESTROY
    if (!haveDestroyCards(player, script.getEffect().getDestroy()))
    {
        return false;   
    }

    return true;
}

bool GameState::canRunScriptUndo(const PlayerID player, const CardID cardID, const Script & script) const
{
    // if we no longer have the resource that this produced we can't undo it
    if (!getResources(player).has(script.getEffect().getReceive()))
    {
        return false;
    }

    // check if all the units created by this script are still alive to be destroyed
    for (const auto & createdCardID : getCardByID(cardID).getCreatedCardIDs())
    {
        if (getCardByID(createdCardID).isDead())
        {
            return false;
        }
    }

    return true;
}

bool GameState::haveDestroyCards(const PlayerID player, const std::vector<DestroyDescription> & desroyDescriptions) const
{
    for (const auto & destroyDescription : desroyDescriptions)
    {
        const PlayerID cardOwner    = destroyDescription.getOwn() ? player : getEnemy(player);
        CardID haveDestroyType      = 0;

        for (const auto & cardID : getCardIDs(cardOwner))
        {
            const Card & card = getCardByID(cardID);
            if (card.getType().getID() == destroyDescription.getTypeID() && card.meetsCondition(destroyDescription.getCondition()))
            {
                haveDestroyType++;
            }

            if (haveDestroyType >= destroyDescription.getMultiple())
            {
                break;
            }
        }

        if (haveDestroyType < destroyDescription.getMultiple())
        {
            return false;
        }
    }
    
    return true;
}


void GameState::runScript(const CardID cardID, const Script & script, size_t scriptType)
{
    Card & card = _getCardByID(cardID);
    const PlayerID player = card.getPlayer();

    if (script.hasManaCost())
    {
        // SUBTRACT MANA COST
        PRISMATA_ASSERT(getResources(player).has(script.getManaCost()), "Do not have cost to run this script");
        _getResources(player).subtract(script.getManaCost());
    }

    // SUBTRACT SAC COST CARDS
    if (script.hasSacCost())
    {
        std::vector<CardID> cardsToSac;
        cardsToSac.reserve(5);
        for (const auto & sacDescription : script.getSacCost())
        {
            getCardsToSac(player, sacDescription, cardsToSac);
            for (const auto & sacID : cardsToSac)
            {
                killCardByID(sacID, CauseOfDeath::AbilitySacCost);
                card.addKilledCardID(sacID);
            }

            cardsToSac.clear();
        }
    }
    
    // RECEIVE MANA 
    _getResources(player).add(script.getEffect().getReceive());

    // GIVE MANA
    _getResources(getEnemy(player)).add(script.getEffect().getGive());

    // PROCESS RESONATE EFFECTS
    if (script.hasResonate())
    {
        const size_t resonateSize = numCardsOfType(player, script.getResonateEffect().getResonateType(), true);
        
        for (size_t r(0); r < resonateSize; ++r)
        {
            // RECEIVE MANA 
            _getResources(player).add(script.getResonateEffect().getReceive());

            // GIVE MANA
            _getResources(getEnemy(player)).add(script.getResonateEffect().getGive());
        }
    }
        
    // CREATE CARDS
    // if this card has already created cards this turn and then been undone, just revive those cards instead
    if (card.getCreatedCardIDs().size() > 0)
    {
        for (const auto & cardID : card.getCreatedCardIDs())
        {
            m_cards.undoKill(cardID);
        }
    }
    else
    {
        // otherwise we need to create those cards from scratch
        for (const auto & createDescription : script.getEffect().getCreate())
        {
            CardType createType = createDescription.getType();
            PlayerID createPlayer = createDescription.getOwn() ? player : getEnemy(player);
            
            for (size_t m(0); m< createDescription.getMultiple(); ++m)
            {
                int cardCreationMethod = (scriptType == ScriptTypes::BuyScript) ? CardCreationMethod::BuyScript : CardCreationMethod::AbilityScript;

                Card toAdd(createType, createPlayer, cardCreationMethod, createDescription.getBuildTime(), createDescription.getLifespan());

                Card & added = m_cards.addCard(toAdd);

                // if this isn't a begin turn script, note that this card created this cardID so we can undo it easily later
                if (scriptType != ScriptTypes::BeginTurnScript)
                {
                    _getCardByID(cardID).addCreatedCardID(added.getID());
                }
            }
        }
    }
            
    // DESTROY CARDS
    for (const auto & destroyDescription : script.getEffect().getDestroy())
    {
        // get the candidate cards that we can destroy with this ability
        std::vector<CardID> cardsToDestroy;
        getCardsToDestroy(player, destroyDescription, cardsToDestroy);

        // sort them in the correct order that the engine should destroy them
        std::sort(cardsToDestroy.begin(), cardsToDestroy.end(), DestroyCardCompare(*this));

        PRISMATA_ASSERT(destroyDescription.getMultiple() <= cardsToDestroy.size(), "We don't have enough of this card type to destroy");

        // kill the first m units in the sorted array
        for (CardID m(0); m < destroyDescription.getMultiple(); ++m)
        {
            killCardByID(cardsToDestroy[m], CauseOfDeath::Sniped);
            _getCardByID(cardID).addKilledCardID(cardsToDestroy[m]);
        }
    }
    

    if (scriptType == ScriptTypes::AbilityScript)
    {
        _getCardByID(cardID).useAbility();
    }

    // KILL CARD IF THE SCRIPT REQUIRES SAC
    if (script.isSelfSac() || _getCardByID(cardID).isDead())
    {
        killCardByID(cardID, CauseOfDeath::SelfSac);
    }
}

void GameState::runScriptUndo(const CardID cardID, const Script & script, size_t scriptType)
{
    const PlayerID player = getCardByID(cardID).getPlayer();
    Card & card = _getCardByID(cardID);

    if (script.hasManaCost())
    {
        _getResources(player).add(script.getManaCost());
    }

    // GET MANA
    _getResources(player).subtract(script.getEffect().getReceive());

    // GIVE MANA
    _getResources(getEnemy(player)).subtract(script.getEffect().getGive());

    // PROCESS RESONATE EFFECTS
    if (script.hasResonate())
    {
        const size_t resonateSize = numCardsOfType(player, script.getResonateEffect().getResonateType(), true);

        for (size_t r(0); r < resonateSize; ++r)
        {
            // GET MANA
            _getResources(player).subtract(script.getResonateEffect().getReceive());

            // GIVE MANA
            _getResources(getEnemy(player)).subtract(script.getResonateEffect().getGive());
        }
    }
    
    // REVIVE ALL THE CARDS KILLED BY THIS CARD
    for (const auto & cardID : card.getKilledCardIDs())
    {
        m_cards.undoKill(cardID);
    }

    // KILL ALL CARDS CREATED BY THIS CARD
    for (const auto & cardID : card.getCreatedCardIDs())
    {
        if (scriptType == ScriptTypes::BuyScript)
        {
            m_cards.removeLiveCardByID(cardID);
        }
        else
        {
            killCardByID(cardID, CauseOfDeath::UndoCreate);
        }
    }

    // resurrect the card if it died from its action
    if (scriptType == ScriptTypes::AbilityScript)
    {
        bool cardDied = card.isDead();
        card.undoUseAbility();

        if (cardDied)
        {
            m_cards.undoKill(cardID);
        }

        // undo the target effects of this ability, if it has any
        undoTargetAbility(card);
    }
}


void GameState::getCardsToSac(const PlayerID abilityOwner, const SacDescription & sacDescription, std::vector<CardID> & cardsToSac) const
{
    const CardID sacTypeID = sacDescription.getTypeID();
    const CardID mult = sacDescription.getMultiple();

    // if we just need to sac one card, do it as a special case for speed
    // this is good practice because most sacrifices are single cards
    if (mult == 1)
    {
        bool minSet = false;
        CardID minCardID = 0;

        // get the minimum unit to sac
        for (const auto & cardID : getCardIDs(abilityOwner))
        {
            const Card & card = getCardByID(cardID);
            if (card.getType().getID() == sacTypeID && card.canSac())
            {
                if (!minSet || (card.getID() < minCardID))
                {
                    minSet = true;
                    minCardID = card.getID();
                }
            }
        }

        PRISMATA_ASSERT(minSet, "Didn't find a single card to sac");

        cardsToSac.push_back(minCardID);
    }
    // if we need to get more than one card, a sort will be involved
    else
    {
        std::vector<CardID> candidateSacCards;
        candidateSacCards.reserve(10);

        for (const auto & cardID : getCardIDs(abilityOwner))
        {
            const Card & card = getCardByID(cardID);
            if (card.getType().getID() == sacTypeID && card.canSac())
            {
                candidateSacCards.push_back(card.getID());
            }
        }

        PRISMATA_ASSERT(candidateSacCards.size() >= mult, "We didn't find enough of the required card to sac");

        if (candidateSacCards.size() > 1 && candidateSacCards.size() > mult)
        {
            std::sort(candidateSacCards.begin(), candidateSacCards.end());
        }

        for (size_t i(0); i < mult; ++i)
        {
            cardsToSac.push_back(candidateSacCards[i]);
        }
    }
}

void GameState::getCardsToDestroy(const PlayerID abilityOwner, const DestroyDescription & destroyDescription, std::vector<CardID> & cardsToDestroy) const
{
    const PlayerID destroyedCardOwner = destroyDescription.getOwn() ? abilityOwner : getEnemy(abilityOwner);

    // find all the candidate cards we can destroy
    for (const auto & cardID : getCardIDs(destroyedCardOwner))
    {
        const Card & card = getCardByID(cardID);

        if (card.getType() == destroyDescription.getType() && card.meetsCondition(destroyDescription.getCondition()))
        {
            cardsToDestroy.push_back(card.getID());
        }
    }
}

const TurnType GameState::getTurnNumber() const
{
    return m_turnNumber;
}

const PlayerID GameState::getActivePlayer() const
{
    return m_activePlayer;
}

const PlayerID GameState::getInactivePlayer() const
{
    return getEnemy(m_activePlayer);
}

void GameState::killCardByID(const CardID cardID, const int causeOfDeath)
{
    m_cards.killCardByID(cardID, causeOfDeath);
}

const PlayerID GameState::getEnemy(const PlayerID player) const
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    return (player + 1) % 2;
}

bool GameState::isGameOver() const
{
    return m_gameOver;
}

bool GameState::calculateGameOver() const
{
    CardID p1Cards = numCards(Players::Player_One) + numKilledCards(Players::Player_One);
    CardID p2Cards = numCards(Players::Player_Two) + numKilledCards(Players::Player_Two);

    return p1Cards == 0 || p2Cards == 0;
}

void GameState::beginTurn(const PlayerID player)
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    // reset resource that needs to be reset
    _getResources(player).set(Resources::Energy, 0);
    _getResources(player).set(Resources::Blue, 0);
    _getResources(player).set(Resources::Red, 0);
    _getResources(player).set(Resources::Attack, 0);

    // reset card breached
    m_canBreachFrozenCard = false;

    // keep track of all the cards we had at the beginning of the turn
    std::vector<CardID> cardsAtStartOfTurn;
    cardsAtStartOfTurn.reserve(numCards(player));
    for (const auto & cardID : getCardIDs(player))
    {
        cardsAtStartOfTurn.push_back(cardID);
    }

    // do begin turn for dead cards
    for (const auto & cardID : getKilledCardIDs(player))
    {
        _getCardByID(cardID).beginTurn();
    }

    // do beginning of turn upkeep for each card
    for (const auto & cardID : cardsAtStartOfTurn)
    {
        PRISMATA_ASSERT(!getCardByID(cardID).isDead(), "Card is dead?");

        _getCardByID(cardID).beginTurn();

        if (_getCardByID(cardID).isDead())
        {
            killCardByID(cardID, CauseOfDeath::Unknown);
        }
    }

    // do begin own turn scrips for each remaining card that isn't dead
    for (const auto & cardID : cardsAtStartOfTurn)
    {
        const Card & card = _getCardByID(cardID);
        
        // if the card is still alive and can run its own begin turn script, do it
        if (!card.isDead() && card.getType().hasBeginOwnTurnScript() && card.canRunBeginOwnTurnScript())
        {
            runScript(cardID, card.getType().getBeginOwnTurnScript(), ScriptTypes::BeginTurnScript);
            bool scriptKilledCard = getCardByID(cardID).isDead();

            _getCardByID(cardID).runBeginTurnScript();

            if (_getCardByID(cardID).isDead() && !scriptKilledCard)
            {
                killCardByID(cardID, CauseOfDeath::Unknown);
            }
        }
    }
    
    m_cards.removeKilledCards();
}

void GameState::beginPhase(const PlayerID player, const int newPhase)
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    m_activePlayer = player;
    m_activePhase = newPhase;

    switch (m_activePhase)
    {
        case Phases::Defense:
        {
            if (getAttack(getEnemy(player)) == 0)
            {
                endPhase();
            }
            break;
        }
        case Phases::Swoosh:
        {
            beginTurn(player);
            endPhase();
            break;
        }
        case Phases::Action:    { break; }
        case Phases::Breach:    { break; }
        case Phases::Confirm:   { break; }
        default: { PRISMATA_ASSERT(false , "Trying to start unknown phase."); }
    }
}

void GameState::endPhase()
{
    const PlayerID player = getActivePlayer();
    const PlayerID enemy = getEnemy(player);

    switch (m_activePhase)
    {
        case Phases::Defense:
        {
            // ending a defense phase with enemy damage means DEFENSE and SWOOSH would happen in 1 turn, which is not legal
            PRISMATA_ASSERT(getAttack(enemy) == 0, "Cannot end DEFENSE phase with remaining enemy damage");
        
            // transition into current player's SWOOSH phase
            beginPhase(player, Phases::Swoosh);
            break;
        }
        case Phases::Swoosh:
        {
            // transition into current player's ACTION phase
            beginPhase(player, Phases::Action);
            break;
        }
        case Phases::Action:
        {
            HealthType ourAttack = getAttack(getActivePlayer());

            // if we have more attack than the enemy has available defense we can breach
            if ((ourAttack > 0) && canWipeout(player))
            {
                blockWithAllBlockers(enemy);

                beginPhase(player, Phases::Breach);
                break;
            }

            beginPhase(player, Phases::Confirm);
            break;
        }
        case Phases::Breach:
        {
            // handle the special case where there's an unbreachable card remaining
            if (hasBreachableCard(enemy) && !canBreachEnemyCard(player))
            {
                _getResources(player).set(Resources::Attack, 0);
            }

            if (hasOverkillableCard(enemy) && !canOverkillEnemyCard(player))
            {
                _getResources(player).set(Resources::Attack, 0);
            }

            // this AI engine does not support confirm after breach, so go to enemy's SWOOSH
            beginPhase(player, Phases::Confirm);
            break;
        }
        case Phases::Confirm:
        {
            m_turnNumber++;
            m_cards.removeKilledCards();

            for (const auto & cardID : getCardIDs(player))
            {
                _getCardByID(cardID).endTurn();
            }

            // we re-calculate the gameOver status at the end of the confirm phase for a player
            m_gameOver = calculateGameOver();

            if (isGameOver())
            {
                m_resources[player].set(Resources::Attack, 0);
            }

            // if we have attack value go to enemy DEFENSE phase
            if (getAttack(player) > 0)
            {
                PRISMATA_ASSERT(getAttack(player) < getTotalAvailableDefense(enemy), "We should not have our attack >= enemy defense at end of confirm phase");
                beginPhase(enemy, Phases::Defense);
            }
            // if we have no attack value so go to their SWOOSH phase
            else 
            {
                beginPhase(enemy, Phases::Swoosh);
            }
            break;
        }
        default:
        {
             PRISMATA_ASSERT(false , "Trying to end unknown phase.");
             break;
        }
    }
}

Card & GameState::buyCardByID(const PlayerID player, const CardID cardID)
{
    // subtract the appropriate resource from the current player
    PRISMATA_ASSERT(getResources(player).has(getCardBuyableByID(cardID).getType().getBuyCost()), "Do not have enough resource to buy this card");
    _getResources(player).subtract(getCardBuyableByID(cardID).getType().getBuyCost());

    Card & boughtCard = m_cards.buyCardByID(player, cardID);
    m_lastCardBoughtID = boughtCard.getID();

    // sac the appropriate units
    CardBuyable cardBuyable = getCardBuyableByID(cardID);
    if (cardBuyable.getType().usesBuySac())
    {
        const std::vector<SacDescription> & buySac = cardBuyable.getType().getBuySac();
        std::vector<CardID> cardsToSac;
        cardsToSac.reserve(7);

        // for each type of unit that we have to sac
        for (CardID sacIndex(0); sacIndex < buySac.size(); ++sacIndex)
        {
            getCardsToSac(player, buySac[sacIndex], cardsToSac);

            // sac the units
            for (size_t c(0); c < cardsToSac.size(); ++c)
            {
                killCardByID(cardsToSac[c], CauseOfDeath::BuySacCost);
                boughtCard.addKilledCardID(cardsToSac[c]);
            }

            cardsToSac.clear();
        }
    }
        
    return boughtCard;
}

void GameState::sellCardByID(const PlayerID player, const CardID cardID)
{
    Card & card = _getCardByID(cardID);
    
    _getResources(player).add(card.getType().getBuyCost());

    // undo the buy script
    // this will also kill any cards created by this card
    runScriptUndo(cardID, card.getType().getBuyScript(), ScriptTypes::BuyScript);

    // kill the card itself
    m_cards.sellCardByID(cardID);
}

const CardID GameState::getLastCardBoughtID() const
{
    return m_lastCardBoughtID;
}

bool GameState::canWipeout(const PlayerID player) const
{
    if (getAttack(player) == 0)             { return false; }
    if (getActivePlayer() != player)        { return false; }
    if (getActivePhase() != Phases::Action) { return false; }
    if (numCards(getEnemy(player)) == 0)    { return false; }
    return (getAttack(player) >= getTotalAvailableDefense(getEnemy(player)));
}

bool GameState::doMove(const Move & move)
{
    for (size_t a(0); a<move.size(); ++a)
    {
        bool didAction = doAction(move.getAction(a));
        PRISMATA_ASSERT(didAction, "GameState doMove tried to do an illegal action, a=%d, Action = %s", a, move.getAction(a).toString().c_str());
    }

    return true;
}

// blocks with all of a player's available blockers
// this function is used to kill off a player's blockers if breach will happen
void GameState::blockWithAllBlockers(const PlayerID player)
{
    // block with all available enemy blockers
    CardID c(0);
    while (c < numCards(player))
    {
        Card & card = _getCardByID(getCardIDs(player)[c]);

        if (card.canBlock())
        {
            blockWithCard(card);
            m_canBreachFrozenCard = true;
            c = 0;
        }
        else
        {
            ++c;
        }
    }
}

const HealthType GameState::getTotalAvailableDefense(const PlayerID player) const
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    HealthType block = 0;
    for (const auto & cardID : getCardIDs(player))
    {
        const Card & card = getCardByID(cardID);

        if (card.canBlock())
        {
            block += card.currentHealth();
        }
    }

    return block;
}


// carries out the effects of blocking with a given card
void GameState::blockWithCard(Card & card)
{
    //HealthType currentDamage = getAttack(getInactivePlayer());
    HealthType currentDamage = (HealthType)getResources(getEnemy(card.getPlayer())).amountOf(Resources::Attack);
    HealthType currentHealth = card.currentHealth();
    HealthType takeDamage = std::min(currentDamage, currentHealth);

    PRISMATA_ASSERT(takeDamage > 0, "Blocking 0 damage. Current %d incoming attack. This will create an infinite loop. Card=%s", (int)currentDamage, card.toJSONString().c_str());

    card.takeDamage(takeDamage, DamageSource::Block);
    if (card.isDead())
    {
        killCardByID(card.getID(), CauseOfDeath::Blocker);
    }

    _getResources(getEnemy(card.getPlayer())).set(Resources::Attack, currentDamage - takeDamage);
}

// carries out the effects of breaching a given card
void GameState::breachCard(Card & card)
{
    HealthType currentDamage = (HealthType)getResources(getEnemy(card.getPlayer())).amountOf(Resources::Attack);
    HealthType currentHealth = card.currentHealth();
    HealthType takeDamage = std::min(currentDamage, currentHealth);

    card.takeDamage(takeDamage, DamageSource::Breach);
    if (card.isDead())
    {
        killCardByID(card.getID(), CauseOfDeath::Breached);
    }

    _getResources(getEnemy(card.getPlayer())).set(Resources::Attack, currentDamage - takeDamage);
}

void GameState::undoBreachCard(Card & card)
{
    // revive the card if it died
    if (card.isDead())
    {
        m_cards.undoKill(card.getID());
    }

    // add the attack back on for the breaching player
    _getResources(getEnemy(card.getPlayer())).set(Resources::Attack, getAttack(getActivePlayer()) + card.getDamageTaken());
    
    card.undoBreach();
}


const HealthType GameState::getAttack(const PlayerID player) const
{
    return (HealthType)getResources(player).amountOf(Resources::Attack);
}

void GameState::addCard(const PlayerID player, const CardType type, const size_t num, const int creationMethod, const TurnType delay, const TurnType lifespan)
{
    for (size_t i(0); i<num; ++i)
    {
        Card newCard(type, player, creationMethod, delay, lifespan);
        Card c2(newCard);
        Card c3 = newCard;
        m_cards.addCard(newCard);
    }
}

void GameState::addCard(const Card & card)
{
    m_cards.addCard(card);
}

void GameState::addBuyableCardType(const CardType type)
{
    m_cards.addBuyableCardType(type);
}

const Resources & GameState::getResources(const PlayerID player) const
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    return m_resources[player];
}

Resources & GameState::_getResources(const PlayerID player)
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    return m_resources[player];
}

void GameState::setMana(const PlayerID player, const Resources & resource)
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    m_resources[player] = resource;
}

const int GameState::getActivePhase() const
{
    return m_activePhase;
}

const CardID GameState::numCards(const PlayerID player) const
{
    return m_cards.numCards(player);
}

const CardID GameState::numKilledCards(const PlayerID player) const
{
    return m_cards.numKilledCards(player);
}

const CardID GameState::numCardsOfType(const PlayerID player, const CardType type, bool requireActive) const
{
    if (!requireActive)
    {
        return m_cards.getCardTypeCount(player, type);
    }

    CardID num(0);

    for (const auto & cardID : getCardIDs(player))
    {
        const Card & card = getCardByID(cardID);
        if (card.getType() == type)
        {
            if (!requireActive || (requireActive && !card.isUnderConstruction() && !card.isDelayed() && !card.isDead()))
            {
                ++num;
            }
        }
    }

    return num;
}

const CardID GameState::numCompletedCardsOfType(const PlayerID player, const CardType type) const
{
    CardID num(0);

    for (const auto & cardID : getCardIDs(player))
    {
        const Card & card = getCardByID(player);
        if (card.getType() == type && !card.isUnderConstruction() && !card.isDelayed())
        {
            ++num;
        }
    }

    return num;
}

//const Card & GameState::getCard(const PlayerID player, const CardID index) const
//{
//    return _cards.getCard(player, index);
//}
//
//const Card & GameState::getKilledCard(const PlayerID player, const CardID index) const
//{
//    return _cards.getKilledCard(player, index);
//}
//
//Card & GameState::_getCard(const PlayerID player, const CardID index)
//{
//    return _cards.getCard(player, index);
//}
//
//Card & GameState::_getKilledCard(const PlayerID player, const CardID index)
//{
//    return _cards.getKilledCard(player, index);
//}

const Card & GameState::getCardByID(const CardID id) const
{
    return m_cards.getCardByID(id);
}

Card & GameState::_getCardByID(const CardID id)
{
    return m_cards.getCardByID(id);
}

const CardID GameState::numCardsBuyable() const
{
    return m_cards.numCardsBuyable();
}

const CardBuyable & GameState::getCardBuyableByID(const CardID cardID) const
{
    return m_cards.getCardBuyableByID(cardID);
}

const CardBuyable & GameState::getCardBuyableByIndex(const CardID index) const
{
    return m_cards.getCardBuyableByIndex(index);
}

const CardBuyable & GameState::getCardBuyableByType(const CardType type) const
{
    return m_cards.getCardBuyableByType(type);
}

CardBuyable & GameState::_getCardBuyableByID(const CardID cardID)
{
    return m_cards.getCardBuyableByID(cardID);
}

CardBuyable & GameState::_getCardBuyableByIndex(const CardID index)
{
    return m_cards.getCardBuyableByIndex(index);
}

bool GameState::canBreachFrozenCard() const
{
    return m_canBreachFrozenCard;
}

const PlayerID GameState::winner() const
{
    if (!isGameOver())
    {
        return Players::Player_None;
    }

    if (numCards(Players::Player_One) + numKilledCards(Players::Player_One) == 0)
    {
        return Players::Player_Two;
    }

    if (numCards(Players::Player_Two) + numKilledCards(Players::Player_Two) == 0)
    {
        return Players::Player_One;
    }

    return Players::Player_None;
}

bool GameState::hasOverkillableCard(const PlayerID player) const
{
    for (const auto & cardID : getCardIDs(player))
    {
        // every card must be overkillable or we can't overkill anything
        if (!getCardByID(cardID).isOverkillable())
        {
            return false;
        }
    }

    return true;
}

bool GameState::canOverkillEnemyCard(const PlayerID player) const
{
    bool canOverkill = false;
    const PlayerID enemy(getEnemy(player));
    for (const auto & cardID : getCardIDs(enemy))
    {
        const Card & card = getCardByID(cardID);

        // every card must be overkillable or we can't overkill anything
        if (!card.isOverkillable())
        {
            return false;
        }

        if (card.canOverkillFor(getAttack(player)))
        {
            canOverkill = true;
        }
    }

    return canOverkill;
}

bool GameState::canBreachEnemyCard(const PlayerID player) const
{
    const PlayerID enemy(getEnemy(player));
    for (const auto & cardID : getCardIDs(enemy))
    {
        if (getCardByID(cardID).canBreachFor(getAttack(player)))
        {
            return true;
        }
    }

    return false;
}

bool GameState::hasBreachableCard(const PlayerID player) const
{
    for (const auto & cardID : getCardIDs(player))
    {
        if (getCardByID(cardID).isBreachable())
        {
            return true;
        }
    }

    return false;
}

void GameState::generateLegalActions(std::vector<Action> & actions) const
{
    actions.clear();
    Action endPhase(getActivePlayer(), ActionTypes::END_PHASE, 0);
    const PlayerID player = getActivePlayer();
    const PlayerID enemy = getEnemy(player);

    switch (getActivePhase())
    {
        case Phases::Defense:
        {
            // add all legal blocking assignments
            for (const auto & cardID : getCardIDs(player))
            {
                const Action block(player, ActionTypes::ASSIGN_BLOCKER, cardID);

                if (isLegal(block))
                {
                    actions.push_back(block);
                }
            }
            
            break;
        }
        case Phases::Action:
        {
            // add all legal purchases
            for (CardID cb(0); cb < numCardsBuyable(); ++cb)
            {
                // temp: don't buy forcefields since they 'ruin' the game
                if (getCardBuyableByIndex(cb).getType().getName().compare("Blood Barrier") == 0)
                {
                    continue;
                }

                const Action buy(player, ActionTypes::BUY, getCardBuyableByIndex(cb).getType().getID());

                if (isLegal(buy))
                {
                    actions.push_back(buy);
                }
            }

            // add all legal Card clicks
            for (const auto & cardID : getCardIDs(player))
            {
                const Card & card = getCardByID(cardID);
                
                // special case for targeted abilities
                if (card.getType().hasTargetAbility())
                {
                    // we must find all legal enemy targets for their ability
                    for (const auto & enemyCardID : getCardIDs(enemy))
                    {
                        const Card & enemyCard = getCardByID(enemyCardID);
                        
                        const Action targetAction(player, card.getType().getActionType(), card.getID(), enemyCard.getID());

                        if (isLegal(targetAction))
                        {
                            actions.push_back(targetAction);
                        }
                    }
                }
                else if (card.getType().hasAbility())
                {
                    const Action ability(player, ActionTypes::USE_ABILITY, card.getID());

                    if (isLegal(ability))
                    {
                        actions.push_back(ability);
                    }
                }
            }

            // add all legal frontline kill actions
            for (const auto & cardID : getCardIDs(enemy))
            {
                const Action frontline(player, ActionTypes::ASSIGN_FRONTLINE, cardID);

                if (isLegal(frontline))
                {
                    actions.push_back(frontline);
                }
            }
            break;
        }
        case Phases::Breach:
        {
            // add all legal breach actions
            for (const auto & cardID : getCardIDs(enemy))
            {
                const Action breach(player, ActionTypes::ASSIGN_BREACH, cardID);

                if (isLegal(breach))
                {
                    actions.push_back(breach);
                }
            }

            // if there were no legal actions in breach phase and we can't end the phase we must be in a frozen card only situation
            if (actions.empty() && !isLegal(endPhase))
            {
                // find the card that are frozen and add unfreezing them to the action vector
                for (const auto & cardID : getCardIDs(enemy))
                {
                    const Action undoFreeze(player, ActionTypes::UNDO_CHILL, cardID);

                    if (isLegal(undoFreeze))
                    {
                        actions.push_back(undoFreeze);
                    }
                }
            }
            break;
        }
    }

    if (actions.empty())
    {
        PRISMATA_ASSERT(isLegal(endPhase), "End phase isn't legal");
        actions.push_back(endPhase);
    }
}

bool GameState::haveSacCost(const PlayerID player, const std::vector<SacDescription> & sacCost) const
{
    for (CardID sacIndex(0); sacIndex < sacCost.size(); ++sacIndex)
    {
        // determine how many of that type we have available to sac
        const CardID sacTypeID = sacCost[sacIndex].getTypeID();
        const CardID mult = sacCost[sacIndex].getMultiple();
        CardID haveSaccableType = 0;

        for (const auto & cardID : getCardIDs(player))
        {
            const Card & card = getCardByID(cardID);
            if (card.getType().getID() == sacTypeID && card.canSac())
            {
                haveSaccableType++;
            }

            if (haveSaccableType >= mult)
            {
                break;
            }
        }

        // if we don't have enough of that type we can't buy the card
        if (haveSaccableType < mult)
        {
            return false;
        }
    }

    return true;
}

void GameState::setStartingState(const PlayerID startPlayer, const CardID numDominionCards)
{   
    // Add base set cards
    for (size_t c(0); c<CardTypes::GetBaseSetCardTypes().size(); ++c)
    {
        addBuyableCardType(CardTypes::GetBaseSetCardTypes()[c]);
    }
    
    std::vector<size_t> pool;
    for (size_t c(0); c<CardTypes::GetDominionCardTypes().size(); ++c)
    {
        pool.push_back(c);
    }

    for (size_t c(0); c<numDominionCards; ++c)
    {
        size_t r = rand() % pool.size();
        addBuyableCardType(CardTypes::GetDominionCardTypes()[pool[r]]);
        std::swap(pool[r], pool.back());
        pool.pop_back();
    }
    
    if (CardTypes::CardTypeExists("Drone"))     addCard(startPlayer, CardTypes::GetCardType("Drone"),    6, CardCreationMethod::Manual, 0, 0);
	if (CardTypes::CardTypeExists("Engineer"))  addCard(startPlayer, CardTypes::GetCardType("Engineer"), 2, CardCreationMethod::Manual, 0, 0);

	if (CardTypes::CardTypeExists("Drone"))     addCard(getEnemy(startPlayer), CardTypes::GetCardType("Drone"), 7, CardCreationMethod::Manual, 0, 0);
	if (CardTypes::CardTypeExists("Engineer"))  addCard(getEnemy(startPlayer), CardTypes::GetCardType("Engineer"), 2, CardCreationMethod::Manual, 0, 0);

    beginPhase(startPlayer, Phases::Swoosh);
}

void  GameState::manuallySetAttack(const PlayerID player, const HealthType attackAmount)
{
    m_resources[player].set(Resources::Attack, attackAmount);
}

std::string GameState::getStateString() const
{
    std::stringstream ss;
    
    for (size_t cb(0); cb<numCardsBuyable(); ++cb)
    {
        CardBuyable cardBuyable = getCardBuyableByIndex(cb);

        ss << "Buyable: " << cardBuyable.getType().getName() << "\n";
    }

    for (PlayerID p(0); p<2; ++p)
    {
        for (const auto & cardID : getCardIDs(p))
        {
            ss << (int)p << " " << getCardByID(cardID).getType().getName() << " " << (int)getCardByID(cardID).currentHealth() << std::endl;
        }
    }

    return ss.str();
}

bool GameState::isTargetAbilityCardClicked() const
{
    return m_targetAbilityCardClicked;
}

const Card & GameState::getTargetAbilityCardClicked() const
{
    return getCardByID(m_targetAbilityCardID);
}

const CardID GameState::getIsomorphicCardID(const Card & card) const
{
    for (const auto & cardID : getCardIDs(card.getPlayer()))
    {
        if (getCardByID(cardID).isIsomorphic(card))
        {
            return cardID;
        }
    }

    PRISMATA_ASSERT(false, "Isomorphic Card not found: %s", card.toJSONString().c_str());
    return -1;
}

void GameState::addCardBuyable(const CardType type)
{
    m_cards.addBuyableCardType(type);
}

bool GameState::isBuyable(const PlayerID player, const CardType type) const
{
    for (size_t i(0); i < numCardsBuyable(); ++i)
    {
        const CardBuyable & cb = getCardBuyableByIndex(i);

        if (cb.getType() ==  type && cb.getMaxSupply(player) > 0)
        {
            return true;
        }
    }

    return false;
}

bool GameState::isIsomorphic(const GameState & otherState) const
{
    if (getActivePhase() != otherState.getActivePhase())
    {
        return false;
    }

    if (getActivePlayer() != otherState.getActivePlayer())
    {
        return false;
    }

    for (PlayerID p(0); p<2; ++p)
    {
        if (!isPlayerIsomorphic(otherState, p))
        {
            return false;
        }
    }
    
    return true;
}

bool GameState::isPlayerIsomorphic(const GameState & otherState, const PlayerID playerID) const
{
    if (numCards(playerID) != otherState.numCards(playerID))
    {
        return false;
    }

    if (getResources(playerID) != otherState.getResources(playerID))
    {
        return false;
    }
    
    // add all cards in the other state to a set
    std::set<CardID> otherCards;
    for (const auto & cardID : otherState.getCardIDs(playerID))
    {
        otherCards.insert(cardID);
    }

    // for each card in this state, see if it has an isomorphic card in the other state
    for (const auto & cardID : getCardIDs(playerID))
    {
        bool found = false;
        for (const auto & otherCardID : otherCards)
        {
            if (getCardByID(cardID).isIsomorphic(otherState.getCardByID(otherCardID)))
            {
                otherCards.erase(otherCardID);
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    PRISMATA_ASSERT(otherCards.size() == 0, "If otherCards.size() isn't 0, states are not isomorphic");

    return true;
}

const size_t GameState::getMemoryUsed() const
{
    size_t size = sizeof(GameState);
    
    return sizeof(GameState) + m_cards.getMemoryUsed();
}

void GameState::manuallySetMana(const PlayerID player, const Resources & resource)
{
    m_resources[player] = resource;
}

Action GameState::getClickAction(const Card & card) const
{
    PlayerID player = getActivePlayer();
    int phase = getActivePhase();
    bool targetAbilityCardClicked = isTargetAbilityCardClicked();

    if (phase == Phases::Action)
    {
        if (!targetAbilityCardClicked)
        {
            if (player == card.getPlayer())
            {
                if (card.isSellable())
                {
                    return Action(player, ActionTypes::SELL, card.getID());
                }

                if (card.getStatus() == CardStatus::Assigned)
                {
                    return Action(player, ActionTypes::UNDO_USE_ABILITY, card.getID());
                }
                else
                {
                    return Action(player, ActionTypes::USE_ABILITY, card.getID());
                }
            }
            else
            {
                if (card.currentChill() > 0)
                {
                    return Action(player, ActionTypes::UNDO_CHILL, card.getID());
                }

                if (card.getType().isFrontline())
                {
                    return Action(player, ActionTypes::ASSIGN_FRONTLINE, card.getID());
                }

                return Action(player, ActionTypes::WIPEOUT, card.getID());
            }
        }
        else
        {
            const Card & targetAbilityCard = getTargetAbilityCardClicked();
            return Action(player, targetAbilityCard.getType().getTargetAbilityType(), targetAbilityCard.getID(), card.getID());
        }
    }
    else if (phase == Phases::Defense)
    {
        return Action(card.getPlayer(), ActionTypes::ASSIGN_BLOCKER, card.getID());
    }
    else if (phase == Phases::Breach)
    {
        if (player != card.getPlayer())
        {
            if (card.wasBreached())
            {
                return Action(player, ActionTypes::UNDO_BREACH, card.getID());
            }

            if (card.isFrozen() && !canBreachFrozenCard())
            {
                return Action(player, ActionTypes::UNDO_CHILL, card.getID());
            }

            return Action(player, ActionTypes::ASSIGN_BREACH, card.getID());
        }

        return Action(card.getPlayer(), ActionTypes::USE_ABILITY, card.getID());   
    }

    return Action();
}

const CardIDVector & GameState::getCardIDs(const PlayerID player) const
{
    return m_cards.getCardIDs(player);
}

const CardIDVector & GameState::getKilledCardIDs(const PlayerID player) const
{
    return m_cards.getKilledCardIDs(player);
}

std::string GameState::toJSONString() const
{
    std::stringstream ss;
    ss << "{";
    ss << "\"whiteMana\":\"" << getResources(0).getString() << "\", \n";
    ss << "\"blackMana\":\"" << getResources(1).getString() << "\", \n";
    ss << "\"turn\":" << (int)getActivePlayer() << ", \n";
    ss << "\"phase\":";

    if (getActivePhase() == Phases::Action)
    {
        ss << "\"action\", \n";
    }
    else if (getActivePhase() == Phases::Breach)
    {
        ss << "\"breach\", \n";
    }
    else if (getActivePhase() == Phases::Defense)
    {
        ss << "\"defense\", \n";
    }

    ss << "\"cards\":[";

    for (size_t i(0); i < numCardsBuyable(); ++i)
    {
        ss << "\"" << getCardBuyableByIndex(i).getType().getUIName() << "\"";
        if (i < numCardsBuyable() - 1)
        {
            ss << ", ";
        }
    }   ss << "], \n";

    ss << "\"whiteTotalSupply\":[";
    for (size_t i(0); i < numCardsBuyable(); ++i)
    {
        ss << getCardBuyableByIndex(i).getMaxSupply(0);

        if (i < numCardsBuyable() - 1)
        {
            ss << ", ";
        }
    }   ss << "], \n";

    ss << "\"blackTotalSupply\":[";
    for (size_t i(0); i < numCardsBuyable(); ++i)
    {
        ss << getCardBuyableByIndex(i).getMaxSupply(1);

        if (i < numCardsBuyable() - 1)
        {
            ss << ", ";
        }
    }   ss << "], \n";

    ss << "\"whiteSupplySpent\":[";
    for (size_t i(0); i < numCardsBuyable(); ++i)
    {
        ss << (getCardBuyableByIndex(i).getMaxSupply(0) - getCardBuyableByIndex(i).getSupplyRemaining(0));

        if (i < numCardsBuyable() - 1)
        {
            ss << ", ";
        }
    }   ss << "], \n";

    ss << "\"blackSupplySpent\":[";
    for (size_t i(0); i < numCardsBuyable(); ++i)
    {
        ss << (getCardBuyableByIndex(i).getMaxSupply(1) - getCardBuyableByIndex(i).getSupplyRemaining(1));

        if (i < numCardsBuyable() - 1)
        {
            ss << ", ";
        }
    }   ss << "], \n";

    ss << "\"table\":\n[";
    for (PlayerID p(0); p < 2; ++p)
    {
        for (size_t i(0); i < getCardIDs(p).size(); ++i)
        {
            ss << getCardByID(getCardIDs(p)[i]).toJSONString();

            if (p == 0 || (i < getCardIDs(p).size() - 1))
            {
                ss << ", \n";
            }
        }
    }   ss << "\n] \n";

    ss << "}";

    return ss.str();
}