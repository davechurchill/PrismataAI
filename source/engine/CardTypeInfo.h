#pragma once

#include "Common.h"
#include "Resources.h"
#include "Script.h"
#include "SacDescription.h"
#include "Condition.h"
#include <set>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "Action.h"

#include "JSONTools.h"

#define MAX_BUY_SAC 10

namespace Prismata
{
 
class CardTypeInfo
{
    
public:
    
    CardTypeInfo();
    CardTypeInfo(const int id, const std::string & name, const rapidjson::Value & value);

    std::string     cardName;
    std::string     description;
    std::string     rarity;
    std::string     uiName;
    std::string     uiShortName;
    std::string     resonateCardName;
    std::string     targetAction;

    mutable std::vector<SacDescription> buySac;
    mutable std::vector<SacDescription> abilitySac;

    std::vector<CardID> resonatesFromIDs;
    std::vector<CardID> resonatesToIDs;

    Resources            buyCost;
    Resources            abilityCost;
    Resources            produces;
    Resources            resonateProduces;

    Script          abilityScript;     
    Script          buyScript;
    Script          beginOwnTurnScript;

    ScriptEffect    abilityScriptEffect;
    ScriptEffect    buyScriptEffect;
    ScriptEffect    beginOwnTurnScriptEffect;

    CardID          typeID;
    HealthType      attack;
    HealthType      startingHealth;
    HealthType      healthUsed;
    HealthType      healthGained;
    HealthType      healthMax;
    HealthType      targetAmount;
    HealthType      abilityAttackAmount;
    HealthType      beginTurnAttackAmount;
    HealthType      attackGivenToEnemy;
    SupplyType      supply;
    TurnType        buildTime;
    TurnType        lifespan;
    ChargeType      startingCharge;
    CardID          droneBuySacCost;

    ActionID        targetActionType;
    Condition       targetAbilityCondition;

    int             customHeuristicValue;
    
    bool            assignedBlocking;
    bool            defaultBlocking;
    bool            frontline;
    bool            fragile;
    bool            potentiallyMoreAttack;
    bool            isTech;
    bool            isSpell;
    bool            hasAbility;
    bool            hasBeginOwnTurnScript;
    bool            isEconCard;
    bool            isBaseSet;
    bool            isAbilityHealthUserOnly;
    bool            hasCustomHeuristicValue;

    bool operator == (const CardTypeInfo & rhs) const;
};

}