#include "CardTypeInfo.h"

using namespace Prismata;

CardTypeInfo::CardTypeInfo()
{

}

CardTypeInfo::CardTypeInfo(const int id, const std::string & name, const rapidjson::Value & value)
    : CardTypeInfo()
{
    cardName    = name;
    uiName      = name;
    uiShortName = name;
    typeID      = id;

    // CARD RARITY + NAME
    JSONTools::ReadString           ("rarity",                  value, rarity);
    JSONTools::ReadString           ("description",             value, description);
    JSONTools::ReadString           ("UIName",                  value, uiName);
    JSONTools::ReadString           ("UIShortname",             value, uiShortName);

    // PROPERTIES        
    JSONTools::ReadInt<HealthType>  ("toughness",               value, startingHealth);
    JSONTools::ReadInt<HealthType>  ("toughness",               value, healthMax);
    JSONTools::ReadInt<HealthType>  ("HPUsed",                  value, healthUsed);
    JSONTools::ReadInt<HealthType>  ("HPGained",                value, healthGained);
    JSONTools::ReadInt<HealthType>  ("HPMax",                   value, healthMax);
    JSONTools::ReadInt<ChargeType>  ("charge",                  value, startingCharge);
    JSONTools::ReadInt<TurnType>    ("buildTime",               value, buildTime);
    JSONTools::ReadInt<TurnType>    ("lifespan",                value, lifespan);
    JSONTools::ReadIntBool          ("fragile",                 value, fragile);
    JSONTools::ReadIntBool          ("baseSet",                 value, isBaseSet);
    JSONTools::ReadIntBool          ("spell",                   value, isSpell);
    JSONTools::ReadIntBool          ("undefendable",            value, frontline);
    JSONTools::ReadIntBool          ("defaultBlocking",         value, defaultBlocking);
    JSONTools::ReadIntBool          ("assignedBlocking",        value, assignedBlocking);
    JSONTools::ReadIntBool          ("potentiallyMoreAttack",   value, potentiallyMoreAttack);
    JSONTools::ReadString           ("resonate",                value, resonateCardName);
    JSONTools::ReadString           ("goldResonate",            value, resonateCardName);

    // BUYING
    JSONTools::ReadMana             ("buyCost",                 value, buyCost);
    JSONTools::ReadScript           ("buyScript",               value, buyScript);
    JSONTools::ReadScriptEffect     ("buyScript",               value, buyScriptEffect);
    JSONTools::ReadSacDescription   ("buySac",                  value, buySac);
    
    // ABILITIES
    JSONTools::ReadMana             ("abilityCost",             value, abilityCost);
    JSONTools::ReadSacDescription   ("abilitySac",              value, abilitySac);
    JSONTools::ReadScript           ("abilityScript",           value, abilityScript);
    JSONTools::ReadScriptEffect     ("abilityScript",           value, abilityScriptEffect);
    JSONTools::ReadScript           ("beginOwnTurnScript",      value, beginOwnTurnScript);
    JSONTools::ReadScriptEffect     ("beginOwnTurnScript",      value, beginOwnTurnScriptEffect);

    // TARGET ABILITIES
    JSONTools::ReadInt<HealthType>  ("targetAmount",            value, targetAmount);
    JSONTools::ReadString           ("targetAction",            value, targetAction);
    JSONTools::ReadCondition        ("condition",               value, targetAbilityCondition);

    // CUSTOM DESIGNER HEURISTIC
    if (value.HasMember("customScore"))
    {
        JSONTools::ReadInt("customScore", value, customHeuristicValue);
        hasCustomHeuristicValue = true;
    }

    //  CARD CATEGORY
    if (value.HasMember("spell"))
    {
        buildTime               = 0;
    }

    // SET THE NEW SCRIPT PROPERTIES
    buyScript.setSelfSac(isSpell);
    buyScript.setEffect(buyScriptEffect);
    abilityScript.setManaCost(abilityCost);
    abilityScript.setSacCost(abilitySac);
    abilityScript.setHealthUsed(healthUsed);
    abilityScript.setEffect(abilityScriptEffect);
    beginOwnTurnScript.setEffect(beginOwnTurnScriptEffect);

    // SPECIAL CASE FOR DEADEYE OPERATIVE
    if (value.HasMember("abilityNetherfy") && value["abilityNetherfy"].IsBool() && value["abilityNetherfy"].GetBool() == true)
    {
        const std::string deadeyeAbility = "{\"destroy\":[{\"cardName\":\"Drone\", \"owner\":\"opponent\", \"supply\":1, \"condition\":{\"notBlocking\":true}}]}";

        rapidjson::Document document;
        bool parsingFailed = document.Parse(deadeyeAbility.c_str()).HasParseError();

        PRISMATA_ASSERT(!parsingFailed, "Error parsing deadeye ability json");

        abilityScript.setEffect(ScriptEffect(document));
    }

    // ADD RESONATE EFFECT
    if (value.HasMember("resonate"))
    {
        beginOwnTurnScript.setResonateEffect(ScriptEffect::ResonateEffect(resonateCardName, Resources("A")));
    }
    else if (value.HasMember("goldResonate"))
    {
        beginOwnTurnScript.setResonateEffect(ScriptEffect::ResonateEffect(resonateCardName, Resources("1")));
    }

    // Set up some extra variables for constant-time access
    attack += abilityScript.getEffect().getAttackValue();
    attack += beginOwnTurnScript.getEffect().getAttackValue();
    beginTurnAttackAmount += beginOwnTurnScript.getEffect().getAttackValue();
    abilityAttackAmount += abilityScript.getEffect().getAttackValue();

    produces.add(abilityScript.getEffect().getReceive());
    produces.add(beginOwnTurnScript.getEffect().getReceive());

    hasBeginOwnTurnScript = beginOwnTurnScript.hasEffect();
    hasAbility = abilityScript.hasEffect();

    if      (!rarity.compare("legendary"))  { supply = SupplyAmount::Legendary; }
    else if (!rarity.compare("rare"))       { supply = SupplyAmount::Rare; }
    else if (!rarity.compare("normal"))     { supply = SupplyAmount::Normal; }
    else if (!rarity.compare("trinket"))    { supply = SupplyAmount::Trinket; }
    else if (!rarity.compare("unbuyable"))  { supply = SupplyAmount::Unbuyable; }
    else    { PRISMATA_ASSERT(false, "Invalid Rarity: %s", rarity.c_str()); }

    if      (targetAction.compare("snipe") == 0)    { targetActionType = ActionTypes::SNIPE; }
    else if (targetAction.compare("disrupt") == 0)  { targetActionType = ActionTypes::CHILL; }

    if (name.compare("Academy") == 0 || name.compare("Brooder") == 0 || name.compare("Conduit") == 0)
    {
        isTech = true;
    }

    // set whether this is an econ only card to be activated every turn
    // if (uiName.compare("Drone") == 0 || uiName.compare("Doomed Drone") == 0)
    const Resources empty;
    if (hasAbility 
        && abilityScript.getEffect().getReceive().amountOf(Resources::Gold) > 0 
        && abilityCost == empty
        && attack == 0
        && abilitySac.empty())
    {
        isEconCard = true;
    }

    // precompute how many drones we have to sac to buy this card
    for (size_t i(0); i < buySac.size(); ++i)
    {
        const SacDescription & sd = buySac[i];

        if (sd.getCardName().compare("Drone") == 0)
        {
            droneBuySacCost += sd.getMultiple();
        }
    }

    // check if this unit is an ability health user only
    if ((hasAbility || targetActionType != ActionTypes::NONE) && fragile && !defaultBlocking && (startingCharge == 0) &&
        !abilityScript.hasManaCost() && !abilityScript.hasSacCost() && (abilityScript.getHealthUsed() > 0) && !beginOwnTurnScript.hasEffect())
    {
        isAbilityHealthUserOnly = true;
    }
}

bool CardTypeInfo::operator == (const CardTypeInfo & rhs) const
{
    if (cardName != rhs.cardName) return false;
    if (description != rhs.description) return false;
    if (rarity != rhs.rarity) return false;
    if (uiName != rhs.uiName) return false;
    if (uiShortName != rhs.uiShortName) return false;
    if (resonateCardName != rhs.resonateCardName) return false;
    if (targetAction != rhs.targetAction) return false;

    if (buyCost != rhs.buyCost) return false;
    if (abilityCost != rhs.abilityCost) return false;
    if (produces != rhs.produces) return false;
    if (resonateProduces != rhs.resonateProduces) return false;

    if (!(abilityScript == rhs.abilityScript)) return false;
    if (!(buyScript == rhs.buyScript)) return false;
    if (!(beginOwnTurnScript == rhs.beginOwnTurnScript)) return false;
    if (!(abilityScriptEffect == rhs.abilityScriptEffect)) return false;
    if (!(buyScriptEffect == rhs.buyScriptEffect)) return false;
    if (!(beginOwnTurnScriptEffect == rhs.beginOwnTurnScriptEffect)) return false;

    if (attack != rhs.attack) return false;
    if (startingHealth != rhs.startingHealth) return false;
    if (healthUsed != rhs.healthUsed) return false;
    if (healthGained != rhs.healthGained) return false;
    if (healthMax != rhs.healthMax) return false;
    if (targetAmount != rhs.targetAmount) return false;
    if (abilityAttackAmount != rhs.abilityAttackAmount) return false;
    if (beginTurnAttackAmount != rhs.beginTurnAttackAmount) return false;
    if (attackGivenToEnemy != rhs.attackGivenToEnemy) return false;
    if (supply != rhs.supply) return false;
    if (buildTime != rhs.buildTime) return false;
    if (lifespan != rhs.lifespan) return false;
    if (startingCharge != rhs.startingCharge) return false;
    if (droneBuySacCost != rhs.droneBuySacCost) return false;
    if (targetActionType != rhs.targetActionType) return false;
    if (!(targetAbilityCondition == rhs.targetAbilityCondition)) return false;

    if (assignedBlocking != rhs.assignedBlocking) return false;
    if (defaultBlocking != rhs.defaultBlocking) return false;
    if (frontline != rhs.frontline) return false;
    if (fragile != rhs.fragile) return false;
    if (potentiallyMoreAttack != rhs.potentiallyMoreAttack) return false;
    if (isTech != rhs.isTech) return false;
    if (isSpell != rhs.isSpell) return false;
    if (hasAbility != rhs.hasAbility) return false;
    if (hasBeginOwnTurnScript != rhs.hasBeginOwnTurnScript) return false;
    if (isEconCard != rhs.isEconCard) return false;
    if (isBaseSet != rhs.isBaseSet) return false;
    if (isAbilityHealthUserOnly != rhs.isAbilityHealthUserOnly) return false;

    return true;
}