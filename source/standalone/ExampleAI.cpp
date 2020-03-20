#include "AITools.h"

using namespace Prismata;

Move GetRandomAIMove(const GameState & state)
{
    // The move will be stored in a Move object, which is a sequence of actions
    Move move;

    // The player ID of the player to move at the current state
    PlayerID playerID = state.getActivePlayer();

    // Copy the current state so we can modify it while deciding on a move
    GameState currentState(state);

    // A vector of actions that we will store the state's legal actions in
    std::vector<Action> legalActions;

    // construct an end phase action for convenience, it will be used later
    Action endPhase(playerID, ActionTypes::END_PHASE, 0);

    // This will construct a sequence of random legal actions until the turn is passed
    // See Player_Random for an extended version of this
    while (currentState.getActivePlayer() == playerID)
    {
        // clear the legal actions array
        legalActions.clear();

        // ask the state to generate the legal actions possible right now
        currentState.generateLegalActions(legalActions);
        
        // get a random action from the legal actions array
        Action a = legalActions[rand() % legalActions.size()];

        // record the action in the move sequence
        move.addAction(a);

        // perform the action to advance the state
        currentState.doAction(a);
    }

    return move;
}

// See AITools::GetAIMove for a version of this function with proper error handling 
void ExampleAIMain()
{
    // Read the input line from the Prismata game client
    std::string inputString;
    std::getline(std::cin, inputString);

    // This stringstream will construct the response string that we will send back to the client
    std::stringstream aiResponse;

    // Initialize the AI system and record the initialization response message
    std::string initString = AITools::InitializeAI(inputString);
    initString.back() = ',';

    // The initialization repsonse message is used by the Prismata client, so record it
    aiResponse << initString;

    // Record how much time the decision making takes (Prismata client wants this)
    Timer t;
    t.start();

    // Parse the GameState from the string, which we will use to make an AI move
    GameState state = AITools::GetStateFromInitString(initString);

    // Get an AI move for this state and store it in a move object
    Move move = GetRandomAIMove(state);

    // Get the click string that will be sent back to the client to be performed
    std::string clickString = AITools::GetClickString(move, state);

    // Optional: use our system for determining if the AI should resign
    bool aiResign = AITools::PlayerShouldResign(state, state.getActivePlayer());
        
    // Record the time it took to make the decisions
    double elapsed = t.getElapsedTimeInMilliSec();
    
    // Construct the AI move portion of the string response to the game client
    aiResponse << "{ \"aicomment\":\"AI Move Successfully Found\", ";
    aiResponse << "\"airesign\":" << (aiResign ? "true" : "false") << ", ";
    aiResponse << "\"aithinktime\":" << (int)elapsed << ", ";
    aiResponse << "\"aimovesize\":" << move.size() << ", ";
    aiResponse << "\"aiclicks\": " << clickString << "}";

    // Construct the return string, and remove all newline characters
    std::string aiString = aiResponse.str();
    aiString.replace(aiString.begin(), aiString.end(), '\n', ' ');
    aiString.replace(aiString.begin(), aiString.end(), '\r\n', ' ');

    // Print the string to be captured by the Prismata client
    std::cout << aiString << "\n";
}

