#pragma once

#include "Common.h"
#include "CardData.h"
#include "Action.h"
#include "Move.h"

namespace Prismata
{
    
class GameState 
{
    CardData    m_cards;         
    Resources   m_resources[2]              = {};
    TurnType    m_turnNumber                = 0;
    PlayerID    m_activePlayer              = Players::Player_One;
    int         m_activePhase               = Phases::Action;
    CardID      m_lastCardBoughtID          = 0;
    CardID      m_targetAbilityCardID       = 0;
    bool        m_targetAbilityCardClicked  = false;
    bool        m_gameOver                  = false;
    bool        m_canBreachFrozenCard       = false;
    bool        m_containsBaseSet           = false;

    Card &          _getCardByID(const CardID id);
    CardBuyable &   _getCardBuyableByIndex(const CardID index);
    CardBuyable &   _getCardBuyableByID(const CardID cardID);
    Resources &     _getResources(const PlayerID player);
    
    void            addBuyableCardType(const CardType type);
    void            endPhase();
    void            beginPhase(const PlayerID player, const int newPhase);
    void            blockWithCard(Card & card);
    void            blockWithAllBlockers(const PlayerID player);
    void            breachCard(Card & card);
    void            undoBreachCard(Card & card);
    void            runScript(const CardID cardID, const Script & script, size_t scriptType);
    void            runScriptUndo(const CardID cardID, const Script & script, size_t scriptType);
    void            initFromJSON(const rapidjson::Value & value);
    bool            undoTargetAbility(Card & card);
    
    Card &          buyCardByID(const PlayerID player, const CardID cardBuyableIndex);
    void            sellCardByID(const PlayerID player, const CardID cardID);
    bool            haveSacCost(const PlayerID player, const std::vector<SacDescription> & sacCost) const;
    bool            haveDestroyCards(const PlayerID player, const std::vector<DestroyDescription> & desroyDescription) const;
    void            getCardsToDestroy(const PlayerID abilityOwner, const DestroyDescription & destroyDescription, std::vector<CardID> & cardsToDestroy) const;
    void            getCardsToSac(const PlayerID abilityOwner, const SacDescription & destroyDescription, std::vector<CardID> & cardsToSac) const;
    bool            calculateGameOver() const;

public:
    
    GameState();
    GameState(const rapidjson::Value & value);

    bool doAction(const Action & action);
    bool doMove(const Move & move);
    void setStartingState(const PlayerID startPlayer, const CardID numDominionCards);
    void generateLegalActions(std::vector<Action> & actions) const;
    void addCard(const PlayerID player, const CardType type, const size_t num, const int creationMethod, const TurnType delay, const TurnType lifespan);
    void addCard(const Card & card);
    void addCardBuyable(const CardType type);
    void setMana(const PlayerID player, const Resources & resource);
    void killCardByID(const CardID cardID, const int causeOfDeath);
    void beginTurn(const PlayerID player);
    void manuallySetAttack(const PlayerID player, const HealthType attackAmount);
    void manuallySetMana(const PlayerID player, const Resources & resource);

    bool isLegal(const Action & action)                                          const;
    bool isGameOver()                                                            const;
    bool hasBreachableCard(const PlayerID player)                              const;
    bool canBreachEnemyCard(const PlayerID player)                             const;
    bool hasOverkillableCard(const PlayerID player)                            const;
    bool canOverkillEnemyCard(const PlayerID player)                           const;
    bool canRunScript(const PlayerID player, const Script & script)            const;
    bool canRunScriptUndo(const PlayerID player, const CardID card, const Script & script) const;
    bool isIsomorphic(const GameState & other)                                   const;
    bool isPlayerIsomorphic(const GameState & other, const PlayerID player)    const;
    bool isBuyable(const PlayerID player, const CardType type)               const;
    bool canBreachFrozenCard()                                                   const;
    bool canWipeout(const PlayerID player)                                     const;
    bool isTargetAbilityCardClicked()                                            const;
    
    const CardID            numCardsOfType(const PlayerID player, const CardType type, bool requireActive = false) const;
    const CardID            numCards(const PlayerID player)                                       const;
    const CardID            numKilledCards(const PlayerID player)                                 const;
    const CardID            numCompletedCardsOfType(const PlayerID player, const CardType type) const;
    const CardID            numCardsBuyable()                                                       const;
    const CardID            getLastCardBoughtID()                                                   const;
    const CardID            getIsomorphicCardID(const Card & card)                                  const;
    const TurnType          getTurnNumber()                                                         const;
    const PlayerID          getActivePlayer()                                                       const;
    const PlayerID          getInactivePlayer()                                                     const;
    const PlayerID          getEnemy(const PlayerID player)                                       const;
    const PlayerID          winner()                                                                const;
    const int               getActivePhase()                                                        const;
    const HealthType        getAttack(const PlayerID player)                                      const;
    const HealthType        getTotalAvailableDefense(const PlayerID player)                       const;
    const Card &            getCardByID(const CardID id)                                          const;
    const Card &            getTargetAbilityCardClicked()                                           const;
    const CardBuyable &     getCardBuyableByIndex(const CardID index)                             const;
    const CardBuyable &     getCardBuyableByID(const CardID cardID)                               const;
    const CardBuyable &     getCardBuyableByType(const CardType type)                             const;
    const Resources &       getResources(const PlayerID player)                                        const;
    const CardIDVector &    getCardIDs(const PlayerID player)                                     const;
    const CardIDVector &    getKilledCardIDs(const PlayerID player)                               const;
    Action                  getClickAction(const Card & card)                                       const;
    
    std::string             getStateString()                                                        const;
    std::string             toJSONString()                                                          const;
    const size_t            getMemoryUsed()                                                         const;
};

class DestroyCardCompare 
{
    const GameState & _state;

public:

    DestroyCardCompare(const GameState & state)
        : _state(state)
    {
    }

    bool operator() (const CardID c1, const CardID c2) const
    {
        const Card & card1 = _state.getCardByID(c1);
        const Card & card2 = _state.getCardByID(c2);

        const int c1props[6] = {    card1.canBlock() ? 1 : 0,
                                    -card1.getCurrentDelay(),
                                    card1.getCurrentLifespan(), 
                                    card1.getCurrentCharges(), 
                                    card1.currentHealth(),
                                    -card1.currentChill() };

        const int c2props[6] = {    card2.canBlock() ? 1 : 0,
                                    -card2.getCurrentDelay(),
                                    card2.getCurrentLifespan(), 
                                    card2.getCurrentCharges(), 
                                    card2.currentHealth(),
                                    -card2.currentChill() };

        for (size_t i(0); i < 6; ++i)
        {
            if (c1props[i] > c2props[i])
            {
                return true;
            }

            if (c1props[i] < c2props[i])
            {
                return false;
            }
        }

        return c1 < c2;
    }
};

}

