#include "CM_helpers.hpp"
#include "algorithm"
#include <random>

//helper functions

//math and logic
float cm_clamp(float val, float lo = -10.0, float hi = 10.0){
	if (lo > hi) return 0;
    return std::min(std::max(val, lo), hi);
}

float cm_fold(float val, float lo = -10.0, float hi = 10.0){

    if (lo == hi){
        return lo;
    }else if (lo < hi){
        int i = 0;
         while ((val < lo || val > hi) && i < 50){
             i++;
            if (val < lo){
                val = -(val - lo) + lo;
            }else if (val > hi){
                val = hi + (hi - val);
            }
        }
        val = std::max(lo, std::min(val, hi));
        return val;
    }
    return 0.0;

    // faster implementation, thanks Paul!

    //this still has a bug on approaching zero.
    // float turns = (hi - lo == 0.0) ? 0.0 : (val - lo) / ( hi - lo );
    // int iturns = (int)turns;
    // float fracTurn = turns - iturns;
    // if( fracTurn < 0 )
    // {
    //     fracTurn = fracTurn + 1;
    //     iturns = iturns - 1;
    // }
    // if( iturns % 2 )
    //    fracTurn = 1.0 - fracTurn;
    // return (hi-lo) * fracTurn + lo;

}

float cm_gauss(float size){
    return (((float)rand() / (RAND_MAX)) * 2.0 - 1.0) * size;
}
float cm_gauss(float size, float offset){
    return cm_gauss(size) + offset;
}

