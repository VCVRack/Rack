
#include "LFN.h"
#include "asserts.h"
#include "CHB.h"
#include "FunVCOComposite.h"
#include "Gray.h"
#include "Shaper.h"
#include "TestComposite.h"
#include "VocalFilter.h"

#include <set>


template <class Comp>
inline static void test()
{
    std::shared_ptr<IComposite> comp = Comp::getDescription();

    std::set<std::string> names;
    assertEQ(comp->getNumParams(), Comp::NUM_PARAMS);
    assertGT(comp->getNumParams(), 0);
    for (int i = 0; i < comp->getNumParams(); ++i)
    {
        auto config = comp->getParam(i);
        assertLT(config.min, config.max);
        assertLE(config.def, config.max);
        assertGE(config.def, config.min);

        assert(config.name);
        std::string name = config.name;
        assert(!name.empty());

        // make sure they are unique
        assert(names.find(name) == names.end());
        names.insert(name);
    }
}

void testIComposite()
{
    test<FunVCOComposite<TestComposite>>();
    test<LFN<TestComposite>>();
    test<VocalFilter<TestComposite>>();
    test<Shaper<TestComposite>>();
    test<CHB<TestComposite>>();
    test<Gray<TestComposite>>();
}