
#include "asserts.h"
#include "Divider.h"
static void testDiv0()
{
    bool called = false;
    auto lambda = [&]() {
        called = true;
    };
    Divider d;
    d.setup(3, lambda);
    assert(!called);

    // fires on first call
    d.step();
    assert(called);

    called = false;
    d.step();
    assert(!called);
    d.step();
    assert(!called);

    d.step();
    assert(called);
}

void testUtils()
{
    testDiv0();
}