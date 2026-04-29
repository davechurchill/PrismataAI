#pragma once

#include "Common.h"
#include "Player.h"
#include "Timer.h"

namespace Prismata
{

class Player_DirectTacticalSearch : public Player
{
public:
    struct Parameters
    {
        double timeLimitMS = 1000.0;
        size_t actionBeam = 48;
        size_t buyCandidates = 12;
        size_t abilityCandidates = 16;
        size_t targetCandidates = 8;
        size_t maxActions = 120;
        bool enemyThreat = true;
    };

private:
    struct MacroAction
    {
        std::vector<Action> actions;
        double prior = 0;
    };

    struct SearchNode
    {
        GameState state;
        Move move;
        double score = 0;
    };

    Parameters _params;
    Timer      _timer;

    bool timeExpired();
    bool appendAction(GameState & state, Move & move, const Action & action) const;
    bool appendBestTargetOrUndo(GameState & state, Move & move);
    bool appendEndPhase(GameState & state, Move & move) const;

    bool planDefense(GameState & state, Move & move);
    bool planAction(GameState & state, Move & move);
    bool planBreach(GameState & state, Move & move);
    bool appendFallbackTurn(GameState & state, Move & move);

    void collectMacros(const GameState & state, std::vector<MacroAction> & macros);
    void collectBuyMacros(const GameState & state, std::vector<MacroAction> & macros);
    void collectAbilityMacros(const GameState & state, std::vector<MacroAction> & macros);
    void collectFrontlineMacros(const GameState & state, std::vector<MacroAction> & macros);

    bool applyMacro(const SearchNode & node, const MacroAction & macro, SearchNode & result);
    void keepBestNodes(std::vector<SearchNode> & nodes, const size_t maxNodes) const;

    double evaluateState(const GameState & state, const PlayerID player) const;
    double evaluatePlayerPosition(const GameState & state, const PlayerID player) const;
    double estimateEnemyThreat(const GameState & state, const PlayerID enemy, const PlayerID defender) const;
    double evaluateCard(const Card & card, const GameState & state, const PlayerID perspective) const;
    double evaluateCardType(const CardType type, const int depth = 0) const;
    double evaluateScript(const Script & script, const int depth) const;
    double evaluateResources(const Resources & resources) const;
    double scoreBuy(const GameState & state, const CardType type) const;
    double scoreAbility(const GameState & state, const Card & card) const;
    double scoreTarget(const GameState & state, const Card & source, const Card & target) const;
    double scoreBreachTarget(const GameState & state, const Card & target) const;
    double scoreBlockerAfterBlock(const GameState & state, const Action & block) const;

public:
    Player_DirectTacticalSearch(const PlayerID playerID, const Parameters & params);

    void getMove(const GameState & state, Move & move);
    std::string getDescription();
    PlayerPtr clone();
};

}
