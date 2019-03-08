#pragma once

inline float tanhDriveSignal(float x, float drive) {
    x *= drive;

    if(x < -1.3f) {
        return -1.f;
    }
    else if(x < -0.75f) {
        return (x * x + 2.6f * x + 1.69f) * 0.833333f - 1.f;
    }
    else if(x > 1.3f) {
        return 1.f;
    }
    else if(x > 0.75f) {
        return 1.f - (x * x - 2.6f * x + 1.69f) * 0.833333f;
    }
    return x;
}
