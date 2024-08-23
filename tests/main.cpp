//-----------------------------------------------
// Test Headers
//-----------------------------------------------
// .....................................................................
// Uncomment the header you wanna use or make your own
// .....................................................................

#include "CollisionBenchmark.h"

#include <magique/assets/AssetPacker.h>
//#include "CollisionTest.h"
//#include "HashGridTest.h"

int main()
{
    Test test{};
    CompileImage("../../res");
    return test.run();
}