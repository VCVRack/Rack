
#include <cmath>

class MMFilter {
    public:
        enum FilterMode {
		FILTER_MODE_LOWPASS = 0,
		FILTER_MODE_HIGHPASS,
		FILTER_MODE_BANDPASS,
		FILTER_MODE_NOTCH,
		kNumFilterModes
        };
        MMFilter() :

                mode(FILTER_MODE_LOWPASS),
                inputValue(0.0),
                cutoff(0.99),
                resonance(0.0),
                buf0(),
                buf1(),
                buf2(),
                buf3(),
                output(),
                feedbackAmount()
		{
            //calculateFeedbackAmount();
		};
        void setmode(int fMode);
        void setfreq(float freq);
        void setinput(float input);
        void setreso(float reso);
        float getoutput();
    private:

        FilterMode mode;
        float inputValue;
        float cutoff;
	    float resonance;
	    float buf0;
        float buf1;
        float buf2;
        float buf3;
        float output;
        float feedbackAmount;
        inline void calculateFeedbackAmount() {
            feedbackAmount = resonance + resonance / (1.0 - getCalculatedCutoff());
        }
        inline double getCalculatedCutoff() const {
            return fmax(fmin(cutoff, 0.99), 0.01);
        };

};
