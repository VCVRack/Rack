#pragma once

namespace rack {

    /**
     * @brief Base class for all signal processors
     */
    struct DSPEffect {

        /**
         * @brief Method for mark parameters as invalidate to trigger recalculation
         */
        virtual void invalidate() {};


        /**
         * @brief Process one step and return the computed sample
         * @return
         */
        virtual void process() {};
    };

}