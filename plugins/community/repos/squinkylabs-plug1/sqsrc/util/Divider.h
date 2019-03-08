#pragma once

#include <functional>

/**
 * Calls a lambda ever 'n' calls
 * Purpose is to make it simple do run a subset of a plugin every 
 * 'n' cycles.
 *
 * lambda is always called on the first call to step();
 */
class Divider
{
public:
    void setup(int n, std::function<void()> l)
    {
        lambda = l;
        divisor = n;
    }

    void step()
    {
        if (--counter == 0) {
            counter = divisor;
            lambda();
        }
    }

private:
    std::function<void()> lambda = nullptr;
    int divisor = 0;
    int counter = 1;
};