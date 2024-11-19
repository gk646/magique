//-----------------------------------------------
// Test Headers
//-----------------------------------------------
// .....................................................................
// Uncomment the header you want to use and/or add your own
// .....................................................................

// Tests
// #include "CollisionBenchmark.h"
 //#include "CollisionTest.h"
//#include "HashGridTest.h"

// Examples
#include "../examples/headers/MultiplayerExample.h"
//#include "../examples/headers/SteamExample.h"
//#include "../examples/headers/WindowExample.h"
//#include "../examples/headers/UIExample.h"
//#include "../examples/headers/LightingExample.h"
//#include "../examples/headers/PathFindingExample.h"

int main()
{
#ifdef MAGIQUE_EXAMPLE
    Example test{};
    return test.run();
#else
    Test test{};
    return test.run();
#endif
}