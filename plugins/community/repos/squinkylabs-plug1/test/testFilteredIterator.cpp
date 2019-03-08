#include <assert.h>
#include <vector>
#include <map>

#include "asserts.h"
#include "FilteredIterator.h"

void test0()
{
    std::vector<int> v = {6, 11, 56};
    using iter_t = filtered_iterator<int, std::vector<int>::iterator>;

    iter_t::filter_func is_even = [](std::vector<int>::iterator a) {
        int aa = *a;
        return !(aa % 2);
    };

    iter_t  it_beg(v.begin(), v.end(), is_even);
    iter_t  it_end(v.end(), v.end(), is_even);

    assertEQ(std::distance(it_beg, it_end), 2);

    assertEQ(*it_beg, 6);
    ++it_beg;
    assertEQ(*it_beg, 56);
    ++it_beg;
    assert(it_beg == it_end);
}

void test1()
{
    std::vector<int> v = {11, 56};
    using iter_t = filtered_iterator<int, std::vector<int>::iterator>;

    iter_t::filter_func is_even = [](std::vector<int>::iterator a) {
        return !(*a % 2);
    };

    iter_t  it_beg(v.begin(), v.end(), is_even);
    iter_t  it_end(v.end(), v.end(), is_even);

    assertEQ(std::distance(it_beg, it_end), 1);

    assertEQ(*it_beg, 56);
    ++it_beg;
    assert(it_beg == it_end);
}

class C
{
public:
    int pp;
    C(int q) : pp(q)
    {
    }
};

static void test2()
{
    using iterator = std::vector<C>::iterator;
    using iter_f = filtered_iterator<C, std::vector<C>::iterator>;
    std::vector<C> x = {10, 20, 30};

    iterator i = x.begin();
    iter_f f(x.begin(), x.end(), [](iterator) { return true; });

    assertEQ(f->pp, i->pp);
    assertEQ((++f)->pp, (++i)->pp);
    assertEQ((f++)->pp, (i++)->pp);
}


static void testConst()
{
    using iterator = std::vector<C>::iterator;
    using const_iterator = std::vector<C>::const_iterator;

    using iter_f = filtered_iterator<C, const_iterator>;
    const std::vector<C> x = {10, 20, 30};

    const_iterator i = x.begin();
    iter_f f(x.begin(), x.end(), [](const_iterator) { return true; });

    assertEQ(f->pp, i->pp);
    assertEQ((++f)->pp, (++i)->pp);
    assertEQ((f++)->pp, (i++)->pp);
}


static void testMap()
{
    using iterator = std::map<size_t, C>::iterator;
    using const_iterator = std::map<size_t, C>::const_iterator;

    using iter_f = filtered_iterator<C, const_iterator>;
    const std::map<size_t, C> x = {
        {2, 10},
        {11, 20},
        {17, 30}
    };

    iter_f::filter_func lambda = [](const_iterator it) {
        auto c = it->second;
        return c.pp == 20;
    };

    iter_f fbegin(x.begin(), x.end(), lambda);
    iter_f fend(x.end(), x.end(), lambda);

    assertEQ(std::distance(fbegin, fend), 1);
    assertEQ(fbegin->second.pp, 20);
}


void testFilteredIterator()
{
    test0();
    test1();
    test2();
    testConst();
    testMap();
}