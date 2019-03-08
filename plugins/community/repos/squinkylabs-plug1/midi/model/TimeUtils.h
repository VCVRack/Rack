#pragma once

class TimeUtils
{
public:
    static float barToTime(int bar)
    {
        return bar * 4.f;     // for now, 4 q in one bar
    }
    static int timeToBar(float time)
    {
        return (int) std::round(time / 4.f);
    }
};