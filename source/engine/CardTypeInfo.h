#pragma once

#include "Common.h"
#include "Action.h"
#include "Resources.h"
#include "Script.h"
#include "SacDescription.h"
#include "Condition.h"
#include "JSONTools.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include <set>

namespace Prismata
{

class CardTypeInfo
{

public:

    CardTypeInfo();
    CardTypeInfo(const int id, const std::string & name, const rapidjson::Value & value);
    
    std::string     cardName                = "None";
    std::string     description             = "";
    std::string     rarity                  = "unbuyable";
    std::string     uiName                  = "None";
    std::string     uiShortName             = "None";
    std::string     resonateCardName        = "";
    std::string     targetAction            = "";
    CardID          typeID                  = 0;
    HealthType      attack                  = 0;
    HealthType      startingHealth          = 1;
    HealthType      healthUsed              = 0;
    HealthType      healthGained            = 0;
    HealthType      healthMax               = 0;
    HealthType      targetAmount            = 0;
    HealthType      abilityAttackAmount     = 0;
    HealthType      beginTurnAttackAmount   = 0;
    HealthType      attackGivenToEnemy      = 0;
    SupplyType      supply                  = 0;
    TurnType        buildTime               = 1;
    TurnType        lifespan                = 0;
    ChargeType      startingCharge          = 0;
    CardID          droneBuySacCost         = 0;
    EvaluationType  customHeuristicValue    = 0;
    ActionID        targetActionType        = ActionTypes::NONE;

    bool            assignedBlocking        = false;
    bool            defaultBlocking         = false;
    bool            frontline               = false;
    bool            fragile                 = false;
    bool            potentiallyMoreAttack   = false;
    bool            isTech                  = false;
    bool            isSpell                 = false;
    bool            hasAbility              = false;
    bool            hasBeginOwnTurnScript   = false;
    bool            isEconCard              = false;
    bool            isBaseSet               = false;
    bool            isAbilityHealthUserOnly = false;
    bool            hasCustomHeuristicValue = false;

    Resources       buyCost;
    Resources       abilityCost;
    Resources       produces;
    Resources       resonateProduces;
    Script          abilityScript;
    Script          buyScript;
    Script          beginOwnTurnScript;
    ScriptEffect    abilityScriptEffect;
    ScriptEffect    buyScriptEffect;
    ScriptEffect    beginOwnTurnScriptEffect;
    Condition       targetAbilityCondition;

    mutable std::vector<SacDescription> buySac;
    mutable std::vector<SacDescription> abilitySac;

    std::vector<CardID> resonatesFromIDs;
    std::vector<CardID> resonatesToIDs;

    bool operator == (const CardTypeInfo & rhs) const;
};

}