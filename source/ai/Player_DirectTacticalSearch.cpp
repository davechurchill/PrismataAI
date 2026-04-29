#include "Player_DirectTacticalSearch.h"

#include <cmath>

using namespace Prismata;

namespace
{
    const double WinScore = 1000000.0;
}

Player_DirectTacticalSearch::Player_DirectTacticalSearch(const PlayerID playerID, const Parameters & params)
    : _params(params)
{
    m_playerID = playerID;
}

bool Player_DirectTacticalSearch::timeExpired()
{
    return _params.timeLimitMS > 0 && _timer.getElapsedTimeInMilliSec() >= _params.timeLimitMS;
}

bool Player_DirectTacticalSearch::appendAction(GameState & state, Move & move, const Action & action) const
{
    if (move.size() >= _params.maxActions)
    {
        return false;
    }

    if (!state.isLegal(action))
    {
        return false;
    }

    state.doAction(action);
    move.addAction(action);
    return true;
}

bool Player_DirectTacticalSearch::appendEndPhase(GameState & state, Move & move) const
{
    return appendAction(state, move, Action(m_playerID, ActionTypes::END_PHASE, 0));
}

double Player_DirectTacticalSearch::evaluateResources(const Resources & resources) const
{
    static const double weights[Resources::NumTypes] = { 4.0, 1.3, 5.5, 5.2, 5.2, 2.7 };

    double value = 0;
    for (size_t r(0); r < Resources::NumTypes; ++r)
    {
        value += weights[r] * resources.amountOf(r);
    }

    return value;
}

double Player_DirectTacticalSearch::evaluateScript(const Script & script, const int depth) const
{
    if (depth > 2)
    {
        return 0;
    }

    double value = 0;
    const ScriptEffect & effect = script.getEffect();

    value += evaluateResources(effect.getReceive());
    value -= 0.85 * evaluateResources(effect.getGive());
    value -= evaluateResources(script.getManaCost());
    value -= 0.8 * script.getHealthUsed();

    for (const CreateDescription & create : effect.getCreate())
    {
        const double createValue = create.getMultiple() * evaluateCardType(create.getType(), depth + 1);
        const double delayDiscount = 1.0 / (1.0 + 0.25 * create.getBuildTime());
        const double lifespanDiscount = create.getLifespan() == 0 ? 1.0 : std::min(1.0, create.getLifespan() / 4.0);

        value += (create.getOwn() ? 1.0 : -1.0) * createValue * delayDiscount * lifespanDiscount;
    }

    for (const DestroyDescription & destroy : effect.getDestroy())
    {
        const double destroyValue = destroy.getMultiple() * evaluateCardType(destroy.getType(), depth + 1);
        value += destroy.getOwn() ? -destroyValue : destroyValue;
    }

    if (script.isSelfSac())
    {
        value -= 5.0;
    }

    if (script.hasResonate())
    {
        value += 0.5 * evaluateResources(script.getResonateEffect().getReceive());
    }

    return value;
}

double Player_DirectTacticalSearch::evaluateCardType(const CardType type, const int depth) const
{
    if (type == CardTypes::None)
    {
        return 0;
    }

    double value = 2.0;

    if (type.hasCustomHeuristicValue())
    {
        value += 0.35 * type.getCustomHeuristicValue();
    }

    value += 1.35 * type.getStartingHealth();
    value += 2.75 * type.getAttack();
    value += 2.25 * type.getBeginTurnAttackAmount();
    value += 2.35 * type.getAbilityAttackAmount();
    value += 1.5 * type.getHealthGained();
    value -= 0.6 * type.getHealthUsed();

    value += evaluateResources(type.produces());
    value += 0.6 * evaluateResources(type.getCreatedUnitsManaProduced());
    value += 0.75 * evaluateScript(type.getBuyScript(), depth + 1);
    value += 0.95 * evaluateScript(type.getAbilityScript(), depth + 1);
    value += 0.65 * evaluateScript(type.getBeginOwnTurnScript(), depth + 1);

    if (type.isEconCard())       { value += 7.0; }
    if (type.isTech())           { value += 8.0; }
    if (type.isSpell())          { value -= 2.0; }
    if (type.isPromptBlocker())  { value += 3.5; }
    if (type.isFrontline())      { value += 4.0; }
    if (type.isFragile())        { value -= 1.2 * std::max<HealthType>(1, type.getStartingHealth()); }
    if (type.hasTargetAbility()) { value += 5.0 + 0.8 * type.getTargetAbilityAmount(); }
    if (type.usesCharges())      { value += 0.9 * type.getStartingCharge(); }

    const double constructionDiscount = 1.0 / (1.0 + 0.22 * type.getConstructionTime());
    const double lifespanDiscount = type.getLifespan() == 0 ? 1.0 : std::min(1.0, 0.35 + type.getLifespan() / 5.0);

    return std::max(0.1, value * constructionDiscount * lifespanDiscount);
}

double Player_DirectTacticalSearch::evaluateCard(const Card & card, const GameState & state, const PlayerID perspective) const
{
    if (card.isDead())
    {
        return 0;
    }

    const CardType type = card.getType();
    double value = evaluateCardType(type);

    if (type.getStartingHealth() > 0)
    {
        const double healthRatio = static_cast<double>(card.currentHealth()) / std::max<double>(1.0, type.getStartingHealth());
        value *= 0.45 + 0.55 * std::min(1.0, healthRatio);
    }

    if (card.isUnderConstruction())
    {
        value *= 1.0 / (1.0 + 0.3 * card.getConstructionTime());
    }

    if (card.isDelayed())
    {
        value *= 1.0 / (1.0 + 0.2 * card.getCurrentDelay());
    }

    if (card.getCurrentLifespan() > 0)
    {
        value *= std::min(1.0, 0.25 + card.getCurrentLifespan() / 4.0);
    }

    if (card.currentChill() > 0)
    {
        const double frozenRatio = static_cast<double>(card.currentChill()) / std::max<double>(1.0, card.currentHealth());
        value *= std::max(0.2, 1.0 - 0.65 * frozenRatio);
    }

    if (card.getStatus() == CardStatus::Assigned && (type.hasAbility() || type.hasTargetAbility()))
    {
        value -= 0.45 * std::max(1.0, evaluateScript(type.getAbilityScript(), 1));
    }

    if (card.canUseAbility())
    {
        value += 0.55 * std::max(0.0, evaluateScript(type.getAbilityScript(), 1));
    }

    if (card.canBlock())
    {
        value += 1.5 * card.currentHealth();
    }

    if (card.getPlayer() == perspective && type.getAttackGivenToEnemy() > 0)
    {
        value -= 2.5 * type.getAttackGivenToEnemy();
    }

    return value;
}

double Player_DirectTacticalSearch::estimateEnemyThreat(const GameState & state, const PlayerID enemy, const PlayerID defender) const
{
    double threat = 2.5 * state.getAttack(enemy);

    for (const CardID cardID : state.getCardIDs(enemy))
    {
        const Card & card = state.getCardByID(cardID);
        const CardType type = card.getType();

        if (card.isDead() || card.isUnderConstruction())
        {
            continue;
        }

        threat += 2.0 * type.getAttack();
        threat += 1.8 * type.getBeginTurnAttackAmount();

        if (card.canUseAbility())
        {
            threat += 2.2 * type.getAbilityAttackAmount();
            threat += 0.5 * std::max(0.0, evaluateScript(type.getAbilityScript(), 1));
        }

        if (type.hasTargetAbility() && type.getTargetAbilityType() == ActionTypes::SNIPE)
        {
            threat += 3.5 + type.getTargetAbilityAmount();
        }
    }

    const double defense = 1.7 * state.getTotalAvailableDefense(defender);
    return std::max(0.0, threat - defense);
}

double Player_DirectTacticalSearch::evaluatePlayerPosition(const GameState & state, const PlayerID player) const
{
    double value = evaluateResources(state.getResources(player));
    value += 2.0 * state.getTotalAvailableDefense(player);

    for (const CardID cardID : state.getCardIDs(player))
    {
        value += evaluateCard(state.getCardByID(cardID), state, player);
    }

    return value;
}

double Player_DirectTacticalSearch::evaluateState(const GameState & state, const PlayerID player) const
{
    const PlayerID enemy = state.getEnemy(player);

    if (state.isGameOver())
    {
        const PlayerID winner = state.winner();
        if (winner == player)
        {
            return WinScore;
        }

        if (winner == enemy)
        {
            return -WinScore;
        }
    }

    double value = evaluatePlayerPosition(state, player) - 0.92 * evaluatePlayerPosition(state, enemy);
    value += 2.3 * state.getAttack(player);
    value -= 2.0 * state.getAttack(enemy);

    if (_params.enemyThreat)
    {
        value -= 0.45 * estimateEnemyThreat(state, enemy, player);
    }

    return value;
}

double Player_DirectTacticalSearch::scoreBuy(const GameState & state, const CardType type) const
{
    double score = evaluateCardType(type) - 0.75 * evaluateResources(type.getBuyCost());

    if (type.isEconCard() && state.getTurnNumber() < 8)
    {
        score += 8.0;
    }

    if (type.isPromptBlocker() && estimateEnemyThreat(state, state.getEnemy(m_playerID), m_playerID) > 0)
    {
        score += 6.0;
    }

    if (type.getAttack() > 0 || type.getAbilityAttackAmount() > 0 || type.getBeginTurnAttackAmount() > 0)
    {
        score += 0.8 * state.getResources(m_playerID).amountOf(Resources::Attack);
    }

    return score;
}

double Player_DirectTacticalSearch::scoreAbility(const GameState & state, const Card & card) const
{
    double score = evaluateScript(card.getType().getAbilityScript(), 0);
    score += 2.3 * card.getType().getAbilityAttackAmount();

    if (card.getType().hasTargetAbility())
    {
        score += 4.0 + card.getType().getTargetAbilityAmount();
    }

    if (card.getType().isEconCard())
    {
        score += 4.0;
    }

    return score;
}

double Player_DirectTacticalSearch::scoreTarget(const GameState & state, const Card & source, const Card & target) const
{
    double score = evaluateCard(target, state, m_playerID);

    if (source.getType().getTargetAbilityType() == ActionTypes::CHILL)
    {
        score *= target.canBlock() ? 0.55 : 0.2;
        score += 2.0 * std::min(source.getType().getTargetAbilityAmount(), target.currentHealth());
    }
    else if (source.getType().getTargetAbilityType() == ActionTypes::SNIPE)
    {
        score += 0.6 * target.currentHealth();
    }

    return score;
}

double Player_DirectTacticalSearch::scoreBreachTarget(const GameState & state, const Card & target) const
{
    const HealthType damage = std::min(state.getAttack(m_playerID), target.currentHealth());
    double score = evaluateCard(target, state, m_playerID);

    if (target.getType().isFragile())
    {
        score *= 1.2;
    }

    return score * damage / std::max<double>(1.0, target.currentHealth());
}

double Player_DirectTacticalSearch::scoreBlockerAfterBlock(const GameState & state, const Action & block) const
{
    GameState child(state);
    if (!child.isLegal(block))
    {
        return -WinScore;
    }

    child.doAction(block);
    return evaluateState(child, m_playerID);
}

void Player_DirectTacticalSearch::collectBuyMacros(const GameState & state, std::vector<MacroAction> & macros)
{
    std::vector<MacroAction> buys;

    for (CardID i(0); i < state.numCardsBuyable(); ++i)
    {
        const CardType type = state.getCardBuyableByIndex(i).getType();

        if (type.getName().compare("Blood Barrier") == 0)
        {
            continue;
        }

        const Action buy(m_playerID, ActionTypes::BUY, type.getID());
        if (!state.isLegal(buy))
        {
            continue;
        }

        MacroAction macro;
        macro.actions.push_back(buy);
        macro.prior = scoreBuy(state, type);
        buys.push_back(macro);
    }

    std::sort(buys.begin(), buys.end(), [](const MacroAction & a, const MacroAction & b)
    {
        return a.prior > b.prior;
    });
    if (buys.size() > _params.buyCandidates)
    {
        buys.resize(_params.buyCandidates);
    }

    macros.insert(macros.end(), buys.begin(), buys.end());
}

void Player_DirectTacticalSearch::collectAbilityMacros(const GameState & state, std::vector<MacroAction> & macros)
{
    std::vector<MacroAction> abilities;

    for (const CardID cardID : state.getCardIDs(m_playerID))
    {
        const Card & card = state.getCardByID(cardID);
        const Action click(m_playerID, ActionTypes::USE_ABILITY, card.getID());

        if (!state.isLegal(click))
        {
            continue;
        }

        if (!card.getType().hasTargetAbility())
        {
            MacroAction macro;
            macro.actions.push_back(click);
            macro.prior = scoreAbility(state, card);
            abilities.push_back(macro);
            continue;
        }

        GameState clicked(state);
        clicked.doAction(click);

        std::vector<MacroAction> targetMacros;
        const PlayerID enemy = state.getEnemy(m_playerID);
        for (const CardID targetID : clicked.getCardIDs(enemy))
        {
            const Action targetAction(m_playerID, card.getType().getTargetAbilityType(), card.getID(), targetID);
            if (!clicked.isLegal(targetAction))
            {
                continue;
            }

            MacroAction macro;
            macro.actions.push_back(click);
            macro.actions.push_back(targetAction);
            macro.prior = scoreAbility(state, card) + scoreTarget(clicked, card, clicked.getCardByID(targetID));
            targetMacros.push_back(macro);
        }

        std::sort(targetMacros.begin(), targetMacros.end(), [](const MacroAction & a, const MacroAction & b)
        {
            return a.prior > b.prior;
        });
        if (targetMacros.size() > _params.targetCandidates)
        {
            targetMacros.resize(_params.targetCandidates);
        }

        abilities.insert(abilities.end(), targetMacros.begin(), targetMacros.end());
    }

    std::sort(abilities.begin(), abilities.end(), [](const MacroAction & a, const MacroAction & b)
    {
        return a.prior > b.prior;
    });
    if (abilities.size() > _params.abilityCandidates)
    {
        abilities.resize(_params.abilityCandidates);
    }

    macros.insert(macros.end(), abilities.begin(), abilities.end());
}

void Player_DirectTacticalSearch::collectFrontlineMacros(const GameState & state, std::vector<MacroAction> & macros)
{
    std::vector<MacroAction> frontlines;
    const PlayerID enemy = state.getEnemy(m_playerID);

    for (const CardID cardID : state.getCardIDs(enemy))
    {
        const Action frontline(m_playerID, ActionTypes::ASSIGN_FRONTLINE, cardID);
        if (!state.isLegal(frontline))
        {
            continue;
        }

        MacroAction macro;
        macro.actions.push_back(frontline);
        macro.prior = 4.0 + evaluateCard(state.getCardByID(cardID), state, m_playerID);
        frontlines.push_back(macro);
    }

    std::sort(frontlines.begin(), frontlines.end(), [](const MacroAction & a, const MacroAction & b)
    {
        return a.prior > b.prior;
    });
    if (frontlines.size() > _params.targetCandidates)
    {
        frontlines.resize(_params.targetCandidates);
    }

    macros.insert(macros.end(), frontlines.begin(), frontlines.end());
}

void Player_DirectTacticalSearch::collectMacros(const GameState & state, std::vector<MacroAction> & macros)
{
    macros.clear();

    if (state.getActivePhase() != Phases::Action || state.isTargetAbilityCardClicked())
    {
        return;
    }

    collectAbilityMacros(state, macros);
    collectBuyMacros(state, macros);
    collectFrontlineMacros(state, macros);

    MacroAction stop;
    stop.actions.push_back(Action(m_playerID, ActionTypes::END_PHASE, 0));
    stop.prior = evaluateState(state, m_playerID);
    macros.push_back(stop);
}

bool Player_DirectTacticalSearch::applyMacro(const SearchNode & node, const MacroAction & macro, SearchNode & result)
{
    if (node.move.size() + macro.actions.size() > _params.maxActions)
    {
        return false;
    }

    result = node;

    for (const Action & action : macro.actions)
    {
        if (!result.state.isLegal(action))
        {
            return false;
        }

        result.state.doAction(action);
        result.move.addAction(action);
    }

    result.score = evaluateState(result.state, m_playerID) + 0.02 * macro.prior - 0.03 * result.move.size();
    return true;
}

void Player_DirectTacticalSearch::keepBestNodes(std::vector<SearchNode> & nodes, const size_t maxNodes) const
{
    std::sort(nodes.begin(), nodes.end(), [](const SearchNode & a, const SearchNode & b)
    {
        return a.score > b.score;
    });

    if (nodes.size() > maxNodes)
    {
        nodes.resize(maxNodes);
    }
}

bool Player_DirectTacticalSearch::appendBestTargetOrUndo(GameState & state, Move & move)
{
    if (!state.isTargetAbilityCardClicked())
    {
        return false;
    }

    const Card & clicked = state.getTargetAbilityCardClicked();
    const PlayerID enemy = state.getEnemy(m_playerID);
    Action bestAction;
    double bestScore = -WinScore;

    for (const CardID targetID : state.getCardIDs(enemy))
    {
        const Action targetAction(m_playerID, clicked.getType().getTargetAbilityType(), clicked.getID(), targetID);
        if (!state.isLegal(targetAction))
        {
            continue;
        }

        const double score = scoreTarget(state, clicked, state.getCardByID(targetID));
        if (score > bestScore)
        {
            bestScore = score;
            bestAction = targetAction;
        }
    }

    if (bestAction.getType() != ActionTypes::NONE)
    {
        return appendAction(state, move, bestAction);
    }

    const Action undo(m_playerID, ActionTypes::UNDO_USE_ABILITY, clicked.getID());
    return appendAction(state, move, undo);
}

bool Player_DirectTacticalSearch::planDefense(GameState & state, Move & move)
{
    if (state.getActivePhase() != Phases::Defense)
    {
        return true;
    }

    while (state.getActivePlayer() == m_playerID && state.getActivePhase() == Phases::Defense && !timeExpired())
    {
        if (state.isLegal(Action(m_playerID, ActionTypes::END_PHASE, 0)))
        {
            return appendEndPhase(state, move);
        }

        Action bestAction;
        double bestScore = -WinScore;

        for (const CardID cardID : state.getCardIDs(m_playerID))
        {
            const Action block(m_playerID, ActionTypes::ASSIGN_BLOCKER, cardID);
            if (!state.isLegal(block))
            {
                continue;
            }

            const double score = scoreBlockerAfterBlock(state, block);
            if (score > bestScore)
            {
                bestScore = score;
                bestAction = block;
            }
        }

        if (bestAction.getType() == ActionTypes::NONE || !appendAction(state, move, bestAction))
        {
            return false;
        }
    }

    if (state.getActivePhase() == Phases::Defense && state.isLegal(Action(m_playerID, ActionTypes::END_PHASE, 0)))
    {
        return appendEndPhase(state, move);
    }

    return state.getActivePhase() != Phases::Defense;
}

bool Player_DirectTacticalSearch::planAction(GameState & state, Move & move)
{
    if (state.getActivePhase() != Phases::Action)
    {
        return true;
    }

    size_t actionBudget = _params.maxActions > move.size() ? _params.maxActions - move.size() : 0;
    if (actionBudget == 0)
    {
        return false;
    }

    if (state.isTargetAbilityCardClicked())
    {
        if (!appendBestTargetOrUndo(state, move))
        {
            return false;
        }

        actionBudget = _params.maxActions > move.size() ? _params.maxActions - move.size() : 0;
        if (actionBudget == 0)
        {
            return state.getActivePhase() != Phases::Action;
        }
    }

    SearchNode root;
    root.state = state;
    root.score = evaluateState(state, m_playerID);

    std::vector<SearchNode> beam(1, root);
    std::vector<SearchNode> finished;

    for (size_t depth(0); depth < actionBudget && !beam.empty() && !timeExpired(); ++depth)
    {
        std::vector<SearchNode> next;

        for (const SearchNode & node : beam)
        {
            if (node.state.getActivePhase() != Phases::Action)
            {
                finished.push_back(node);
                continue;
            }

            std::vector<MacroAction> macros;
            collectMacros(node.state, macros);

            for (const MacroAction & macro : macros)
            {
                if (node.move.size() + macro.actions.size() > actionBudget)
                {
                    continue;
                }

                SearchNode child;
                if (!applyMacro(node, macro, child))
                {
                    continue;
                }

                if (child.state.getActivePhase() == Phases::Action)
                {
                    next.push_back(child);
                }
                else
                {
                    finished.push_back(child);
                }
            }
        }

        keepBestNodes(next, _params.actionBeam);
        keepBestNodes(finished, _params.actionBeam);
        beam.swap(next);

        if (!finished.empty() && beam.empty())
        {
            break;
        }
    }

    if (finished.empty())
    {
        return appendEndPhase(state, move);
    }

    keepBestNodes(finished, 1);
    state = finished.front().state;
    move.addMove(finished.front().move);

    return true;
}

bool Player_DirectTacticalSearch::planBreach(GameState & state, Move & move)
{
    if (state.getActivePhase() != Phases::Breach)
    {
        return true;
    }

    while (state.getActivePlayer() == m_playerID && state.getActivePhase() == Phases::Breach && !timeExpired())
    {
        if (state.isLegal(Action(m_playerID, ActionTypes::END_PHASE, 0)))
        {
            return appendEndPhase(state, move);
        }

        const PlayerID enemy = state.getEnemy(m_playerID);
        Action bestAction;
        double bestScore = -WinScore;

        for (const CardID cardID : state.getCardIDs(enemy))
        {
            const Action breach(m_playerID, ActionTypes::ASSIGN_BREACH, cardID);
            if (!state.isLegal(breach))
            {
                continue;
            }

            const double score = scoreBreachTarget(state, state.getCardByID(cardID));
            if (score > bestScore)
            {
                bestScore = score;
                bestAction = breach;
            }
        }

        if (bestAction.getType() == ActionTypes::NONE)
        {
            for (const CardID cardID : state.getCardIDs(enemy))
            {
                const Action undoChill(m_playerID, ActionTypes::UNDO_CHILL, cardID);
                if (state.isLegal(undoChill))
                {
                    bestAction = undoChill;
                    break;
                }
            }
        }

        if (bestAction.getType() == ActionTypes::NONE || !appendAction(state, move, bestAction))
        {
            return false;
        }

        if (bestAction.getType() == ActionTypes::UNDO_CHILL)
        {
            return true;
        }
    }

    if (state.getActivePhase() == Phases::Breach && state.isLegal(Action(m_playerID, ActionTypes::END_PHASE, 0)))
    {
        return appendEndPhase(state, move);
    }

    return state.getActivePhase() != Phases::Breach;
}

bool Player_DirectTacticalSearch::appendFallbackTurn(GameState & state, Move & move)
{
    for (size_t guard(0); guard < _params.maxActions && state.getActivePlayer() == m_playerID && !state.isGameOver(); ++guard)
    {
        if (state.getActivePhase() == Phases::Confirm)
        {
            return appendEndPhase(state, move);
        }

        if (state.getActivePhase() == Phases::Action)
        {
            if (state.isTargetAbilityCardClicked())
            {
                if (!appendBestTargetOrUndo(state, move))
                {
                    return false;
                }
                continue;
            }

            if (!appendEndPhase(state, move))
            {
                return false;
            }
            continue;
        }

        if (state.getActivePhase() == Phases::Defense)
        {
            if (state.isLegal(Action(m_playerID, ActionTypes::END_PHASE, 0)))
            {
                if (!appendEndPhase(state, move))
                {
                    return false;
                }
                continue;
            }

            Action bestBlock;
            double bestScore = -WinScore;
            for (const CardID cardID : state.getCardIDs(m_playerID))
            {
                const Action block(m_playerID, ActionTypes::ASSIGN_BLOCKER, cardID);
                if (!state.isLegal(block))
                {
                    continue;
                }

                const double score = scoreBlockerAfterBlock(state, block);
                if (score > bestScore)
                {
                    bestScore = score;
                    bestBlock = block;
                }
            }

            if (!appendAction(state, move, bestBlock))
            {
                return false;
            }
            continue;
        }

        if (state.getActivePhase() == Phases::Breach)
        {
            if (state.isLegal(Action(m_playerID, ActionTypes::END_PHASE, 0)))
            {
                if (!appendEndPhase(state, move))
                {
                    return false;
                }
                continue;
            }

            const PlayerID enemy = state.getEnemy(m_playerID);
            Action bestBreach;
            double bestScore = -WinScore;
            for (const CardID cardID : state.getCardIDs(enemy))
            {
                const Action breach(m_playerID, ActionTypes::ASSIGN_BREACH, cardID);
                if (state.isLegal(breach))
                {
                    const double score = scoreBreachTarget(state, state.getCardByID(cardID));
                    if (score > bestScore)
                    {
                        bestScore = score;
                        bestBreach = breach;
                    }
                }
                else
                {
                    const Action undoChill(m_playerID, ActionTypes::UNDO_CHILL, cardID);
                    if (bestBreach.getType() == ActionTypes::NONE && state.isLegal(undoChill))
                    {
                        bestBreach = undoChill;
                    }
                }
            }

            if (!appendAction(state, move, bestBreach))
            {
                return false;
            }
            continue;
        }

        return false;
    }

    return state.getActivePlayer() != m_playerID;
}

void Player_DirectTacticalSearch::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _timer.start();

    GameState currentState(state);

    while (currentState.getActivePlayer() == m_playerID && !currentState.isGameOver() && move.size() < _params.maxActions)
    {
        bool planned = false;

        if (currentState.getActivePhase() == Phases::Defense)
        {
            planned = planDefense(currentState, move);
        }
        else if (currentState.getActivePhase() == Phases::Action)
        {
            planned = planAction(currentState, move);
        }
        else if (currentState.getActivePhase() == Phases::Breach)
        {
            planned = planBreach(currentState, move);
        }
        else if (currentState.getActivePhase() == Phases::Confirm)
        {
            planned = appendEndPhase(currentState, move);
        }

        if (!planned || timeExpired())
        {
            appendFallbackTurn(currentState, move);
            break;
        }
    }

    if (move.size() == 0)
    {
        appendFallbackTurn(currentState, move);
    }
}

std::string Player_DirectTacticalSearch::getDescription()
{
    std::stringstream ss;
    ss << m_description << "\n";
    ss << "Direct Tactical Search\n";
    ss << "Time Limit:     " << _params.timeLimitMS << "ms\n";
    ss << "Action Beam:    " << _params.actionBeam << "\n";
    ss << "Buy Candidates: " << _params.buyCandidates << "\n";
    ss << "Ability Cand.:  " << _params.abilityCandidates << "\n";
    ss << "Target Cand.:   " << _params.targetCandidates << "\n";
    ss << "Max Actions:    " << _params.maxActions << "\n";
    ss << "Enemy Threat:   " << (_params.enemyThreat ? "true" : "false");

    return ss.str();
}

PlayerPtr Player_DirectTacticalSearch::clone()
{
    PlayerPtr ret(new Player_DirectTacticalSearch(m_playerID, _params));
    ret->setDescription(m_description);

    return ret;
}
