#include "MoveIterator_PPPortfolio.h"
#include "AllPlayers.h"

using namespace Prismata;

MoveIterator_PPPortfolio::MoveIterator_PPPortfolio(const PlayerID playerID)
    : m_portfolio(PPPhases::NUM_PHASES, std::vector<PPPtr>())
    , m_currentIndex(PPPhases::NUM_PHASES, 0)
    , m_previousMoveChanged(PPPhases::NUM_PHASES, true)
    , m_previousMoveGenerated(PPPhases::NUM_PHASES, Move())
{
    _playerID = playerID;
}

void MoveIterator_PPPortfolio::setState(const GameState & state)
{
    _state = state;

    reset();
}

bool MoveIterator_PPPortfolio::generateNextChild(GameState & child, Move & movePerformed)
{
    while (true)
    {
        movePerformed.clear();

        if (!hasMoreMoves())
        {
            return false;
        }

        child = GameState(_state);
        Action end(child.getActivePlayer(), ActionTypes::END_PHASE, 0);

        // get moves from the partial player children
        for (size_t pp(0); pp < m_currentSequence.size(); ++pp)
        {
            // if we can't re-use the previous move generated, generate it and store it in the previous vector
            if (m_previousMoveChanged[pp])
            {
                m_previousMoveGenerated[pp].clear();
                if (child.getActivePlayer() == _playerID)
                {
                    m_currentSequence[pp]->getMove(child, m_previousMoveGenerated[pp]);
                }

                if (pp == PPPhases::ACTION_BUY && child.getActivePhase() == Phases::Action)
                {
                    child.doAction(end);
                    m_previousMoveGenerated[pp].addAction(end);
                }
            }
            // otherwise we can re-use the previous move, so skip the generation phase and simply apply the previously generated moves
            else
            {
                child.doMove(m_previousMoveGenerated[pp]); 
            }

            // copy the move we generated into the total move vector
            movePerformed.addMove(m_previousMoveGenerated[pp]);

            m_previousMoveChanged[pp] = false;
        }

        // we must end on a CONFIRM phase otherwise there is something wrong with move generation
        PRISMATA_ASSERT(child.getActivePhase() == Phases::Confirm, "Partial player sequence did not end in CONFIRM phase");

        // do the last end phase action and add it to the move
        child.doAction(end);
        movePerformed.addAction(end);

        PPSequence tempSequence = m_currentSequence;

        incrementMove();

        // if we found a move we haven't done before, add it to the vector
        if (std::find(m_movesPerformed.begin(), m_movesPerformed.end(), movePerformed) == m_movesPerformed.end())
        {
            m_movesPerformed.push_back(movePerformed);
            m_previousSequence = tempSequence;
            m_hasDescription = true;
            return true;
        }
    }

    return false;
}

void MoveIterator_PPPortfolio::getRandomMove(const GameState & state, Move & move)
{
    PPSequence randomSequence;

    for (size_t i(0); i < PPPhases::NUM_PHASES; ++i)
    {
        randomSequence[i] = m_portfolio[i][rand() % m_portfolio[i].size()];
    }

    m_previousSequence = randomSequence;
    m_hasDescription = true;

    Player_PPSequence randomSequencePlayer(state.getActivePlayer(), randomSequence);
    randomSequencePlayer.getMove(state, move);
}

void MoveIterator_PPPortfolio::addPartialPlayer(const size_t phase, const PPPtr & player)
{
    PRISMATA_ASSERT(player->playerID() == _playerID, "Player ID of input player does not match the iterator's player");
    m_portfolio[phase].push_back(player);
}

void MoveIterator_PPPortfolio::reset()
{
    _hasMoreMoves = true;
    std::fill(m_currentIndex.begin(), m_currentIndex.end(), 0);
    m_movesPerformed.clear();
    std::fill(m_previousMoveChanged.begin(), m_previousMoveChanged.end(), true);
    std::fill(m_previousMoveGenerated.begin(), m_previousMoveGenerated.end(), Move());

    for (size_t i(0); i<PPPhases::NUM_PHASES; ++i)
    {
        PRISMATA_ASSERT(m_portfolio[i].size() > 0, "Portfolio size 0 for phase %d", i);
        m_currentSequence.setPP(i, m_portfolio[i][0]);
    }
}

void MoveIterator_PPPortfolio::incrementMove(const size_t phase)
{
    // increment the index for this unit
    m_currentIndex[phase] = (m_currentIndex[phase] + 1) % m_portfolio[phase].size();
    m_previousMoveChanged[phase] = true;

    // if the value rolled over, we need to do the carry calculation
    if (m_currentIndex[phase] == 0)
    {
        // if we have space left to increment, do it
        if (phase > 0)
        {
            incrementMove(phase - 1);
        }
        // otherwise we have no more moves
        else
        {
            // stop
            _hasMoreMoves = false;
        }
    }

    m_currentSequence[phase] = m_portfolio[phase][m_currentIndex[phase]];
}

MoveIteratorPtr MoveIterator_PPPortfolio::clone()
{
    MoveIterator_PPPortfolio * temp = new MoveIterator_PPPortfolio(_playerID);

    for (size_t i(0); i < PPPhases::NUM_PHASES; ++i)
    {
        for (size_t j(0); j < m_portfolio[i].size(); ++j)
        {
            temp->addPartialPlayer(i, m_portfolio[i][j]->clone());
        }
    }

    return MoveIteratorPtr(temp);   
}

void MoveIterator_PPPortfolio::setBuyLimits(const BuyLimits & buyLimits)
{
    for (size_t i(0); i<PPPhases::NUM_PHASES; ++i)
    {
        for (size_t j(0); j<m_portfolio[i].size(); ++j)
        {
            BuyLimits limits = m_portfolio[i][j]->getBuyLimits();
            limits.addLimits(buyLimits);
            m_portfolio[i][j]->setBuyLimits(limits);
        }
    }
}

std::string MoveIterator_PPPortfolio::getDescription() 
{ 
    if (!m_hasDescription)
    {
        return std::string("Portfolio has no desc");
    }

    return m_previousSequence.getDescription(); 
}