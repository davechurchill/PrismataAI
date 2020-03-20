#include "PrismataAI.h"
#include "GUIEngine.h"

using namespace Prismata;

int main(int argc, char *argv[])
{
    // Initialize the Prismata Card Library from the JSON library file
    Prismata::InitFromCardLibrary("asset/config/cardLibrary.jso");

    // Parse the AI Parameters from the AI config file
    Prismata::AIParameters::Instance().parseFile("asset/config/config.txt");

    // Construct the GUIEngine object and run the game
    GUIEngine g;
    g.run();

    return 0;
}
