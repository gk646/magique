//-----------------------------------------------
// Test Headers
//-----------------------------------------------
// .....................................................................
// Uncomment the header you want to use or make your own
// .....................................................................

// Tests
// #include "CollisionBenchmark.h"
// #include "CollisionTest.h"
// #include "HashGridTest.h"

// Examples
#include "../examples/headers/MultiplayerExample.h"
//#include "../examples/headers/SteamExample.h"
//#include "../examples/headers/WindowExample.h"
//#include "../examples/headers/UIExample.h"

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