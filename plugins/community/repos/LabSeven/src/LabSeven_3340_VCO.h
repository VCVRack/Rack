#ifndef LABSEVEN_3340_VCO_H
#define LABSEVEN_3340_VCO_H

#include <fstream>
#include <float.h>

#define _USE_MATH_DEFINES
using namespace std;

namespace LabSeven
{
    namespace LS3340
    {
        //Blackman Harris function
        inline double wfBlackmanHarris64bit(double nbr_points, double t, double *parameters = NULL)
        {
            double _2_pi_by_N_1_times_t = (2.0 * M_PI / (nbr_points - 1))*t;
            return 0.35875
                 - 0.48829 * cos(1.0 * _2_pi_by_N_1_times_t)
                 + 0.14128 * cos(2.0 * _2_pi_by_N_1_times_t)
                 - 0.01168 * cos(3.0 * _2_pi_by_N_1_times_t);
        }

        //SINC function
        inline double sinc(double t)
        {
            if(t == 0.0)
            {
                return 1.0;
            }
            else
            {
                double M_PI_t;
                M_PI_t = M_PI * t;
                return sin(M_PI_t) / M_PI_t;
            }
        }

        double *makeOversampledUnintegrated3340ImpulseDbl(int widthSamples = 8, int ovrsampling = 10000)
        {
            //SH pulse specifications:
            //@192kHz: width 8 samples, integrator leakage 1.0-0.27
            //norm 0.27 (coincidence?)
            int nbrSamples = widthSamples*ovrsampling;
            double *pulse = new double[nbrSamples];
            double sum;
            double norm = 0.27; //magic number!

            sum = 0.0;
            for (int i = 0; i < nbrSamples; i++)
            {
                pulse[i] = wfBlackmanHarris64bit(nbrSamples, i) - wfBlackmanHarris64bit(nbrSamples, 0);
                sum += pulse[i];
            }
            for (int i = 0; i < nbrSamples; i++)
            {
                pulse[i] *= norm * (double)ovrsampling / sum;
            }

            return pulse;

            //integration to complete 3340 pulse: s[i] += s[i - 1] * (1.0 - 0.27 / oversampling);
        }

        struct TLS3340VCOFrame
        {
            double square;
            double sawtooth;
            double subosc;
            double noise;
            double triangle;
        };

        struct TLS3340ImpulseParameters
        {
            size_t position;
            double offset;
            double phaseStep;
            double scaling;
        };

        struct TLS3340VCOImpulseLUT
        {
            //use high oversampling factor + next neighbour interpolation for performance
            size_t impulseLengthFrames;
            double oversamplingFactor;
            size_t lutSize;
            double *lut;

            //auxiliary
            double posOversampled;

            TLS3340VCOImpulseLUT()
            {

                impulseLengthFrames      = 8;
                oversamplingFactor       = 10000;
                lutSize                  = impulseLengthFrames * (size_t)oversamplingFactor;

                lut = makeOversampledUnintegrated3340ImpulseDbl(impulseLengthFrames,oversamplingFactor);
            }
            ~TLS3340VCOImpulseLUT()
            {
                delete lut;
            }
            inline double getValAt(double position)//position without oversampling
            {
                //position with oversampling
                posOversampled = position*oversamplingFactor;

                //return 0 if position is out of bounds
                if (posOversampled < 0.0 || posOversampled > lutSize-1) //
                {
                    return 0;
                }

                //next neigbour interpolation
                return lut[(size_t)round(posOversampled)];
            }
        };

        struct TLS3340VCOImpulseTrain
        {
            const static int maxNbrImpulsesPerTrain = 50; //depends on impulse length!
            TLS3340ImpulseParameters train[maxNbrImpulsesPerTrain];

            //runtime stuff
            int trainIndex;
            int nbrActiveImpulses;
            int currentIndex;
            double impulseCounter;

            //needed for the leaky integrator that turns the lut impulse into the 340 impulse
            double integrationBuffer;

            TLS3340VCOImpulseTrain()
            {
                trainIndex        = 0;
                nbrActiveImpulses = 0;
                currentIndex      = 0;
                integrationBuffer = 0.0;
            }

            inline void addImpulse(double offset, double phaseStep, double scaling = 1.0)
            {
                //increment/wrap trainIndex
                trainIndex++;
                if (trainIndex >= maxNbrImpulsesPerTrain) trainIndex = 0;

                //initialize new impulse
                train[trainIndex].position  = 0;
                train[trainIndex].offset    = offset;
                train[trainIndex].phaseStep = phaseStep;
                train[trainIndex].scaling   = scaling;

                //increment/limit nbrActiveImpulses
                nbrActiveImpulses++;
                if (nbrActiveImpulses > maxNbrImpulsesPerTrain)
                    nbrActiveImpulses = maxNbrImpulsesPerTrain;
            }

            inline void getNextSumOfImpulsesAndSawtoothSlope(TLS3340VCOImpulseLUT *lut,
                                                             double &currentSumOfImpulses)//,double &currentSumOfSlopes)
            {
                impulseCounter       = 0.0;
                currentSumOfImpulses = 0.0;

                currentIndex = trainIndex;

                for (int i = 0; i < nbrActiveImpulses; i++)
                {
                    //add contribution of current impulse
                    if (train[currentIndex].phaseStep > 0.0)
                    {
                        currentSumOfImpulses   += train[currentIndex].scaling *
                                                  lut->getValAt(train[currentIndex].position +
                                                                train[currentIndex].offset/train[currentIndex].phaseStep);
                    }

                    //deactivate impulse if it has been completely put out
                    //(can be done this way because this impulse is the oldest active one;
                    //therefore, this happens only while i == nbrActiveImpulses-1)
                    train[currentIndex].position++;
                    if (train[currentIndex].position > lut->impulseLengthFrames-1)
                    {
                        nbrActiveImpulses--;
                    }

                    //prepare currentIndex for next impulse
                    currentIndex--;
                    if (currentIndex < 0) currentIndex += maxNbrImpulsesPerTrain;
                }

                //integration
                currentSumOfImpulses   += integrationBuffer * (1.0 - 0.27);
                integrationBuffer = currentSumOfImpulses;
            }
        };

        struct TLS3340NoiseSource
        {
            private:
                static const size_t dataLengthSamples = 960527;
                float LS3340Noise[dataLengthSamples];
                unsigned long currentPosition;
            public:
                TLS3340NoiseSource()
                {
                    //initialize noise array
                    for (size_t i=0;i<dataLengthSamples;i++)
                    {
                        LS3340Noise[i] = 0.0;
                    }

                    //LOAD NOISE SAMPLE
                    ifstream f;
                    f.open(assetPlugin(plugin, "res/LabSeven_3340_noise.pcm"), ios::binary);
                    f.read((char*)&(LS3340Noise[0]),dataLengthSamples*sizeof(float));
                    f.close();

                    //randomize start position
                    //makes phasing unlikely in case multiple 3340s are used together
                    currentPosition = (unsigned long)round(((double) rand() / (RAND_MAX))*(double)(dataLengthSamples-1));
                }
                inline float getNextNoiseSample()
                {
                    if (dataLengthSamples == 0)
                    {
                        return 0.0;
                    }
                    else
                    {
                        //move/wrap currentPosition
                        currentPosition++;
                        if (currentPosition >= dataLengthSamples)
                            currentPosition -= dataLengthSamples;

                        return LS3340Noise[currentPosition];
                    }
                }
        };

        struct TLS3340VCOSINCLUT
        {
            private:
                int sincTableSize;
                float *sincTable;

                double overSampling;
                double zeroCrossings;
                double posOversampled;
                double middle;
            public:
                inline float getValAt(double posRelMiddleNotOversampled)
                {
                    posOversampled = posRelMiddleNotOversampled * overSampling;
                    int pos = (size_t)round(middle+posOversampled);
                    if (pos >= 0 && pos < sincTableSize) //TODO: This should always be the case but it is not!
                    {
                        return sincTable[pos];
                    }
                    else return 0.0f;
                }
                inline int size(){return sincTableSize;}
                TLS3340VCOSINCLUT(int zeroCrossings, int overSampling)
                {
                    this->overSampling = overSampling;
                    this->zeroCrossings = zeroCrossings;
                    sincTableSize = (zeroCrossings * 2 * overSampling) + 1;
                    middle = floor(sincTableSize/2.0);
                    sincTable = new float[sincTableSize];

                    // BEGIN calculate sinc lut
                    // Based on code by Daniel Werner https://www.experimentalscene.com/articles/minbleps.php
                    double a = (double)-zeroCrossings;
                    double b = (double)zeroCrossings;
                    double r;
                    for(int i = 0; i < sincTableSize; i++)
                    {
                        r = ((double)i) / ((double)(sincTableSize - 1));
                        sincTable[i] = (float)sinc(a + (r * (b - a)));
                        sincTable[i] *= (float)sinc(-1.0 + (r * (2.0))); //Window function is sinc, too!
                    }
                    // END calculate sinc lut

                    //normalize sinc lut
                    double sum = 0.0;
                    for(int i = 0; i < sincTableSize; i++)
                    {
                        sum += sincTable[i];
                    }
                    sum /= overSampling;
                    sum = 1.0/sum;
                    for(int i = 0; i < sincTableSize; i++)
                    {
                        sincTable[i] *= sum;
                    }
                }
                ~TLS3340VCOSINCLUT()
            {
                delete sincTable;
            }
        };

        struct TLS3340VCO
        {
        private:

            TLS3340VCOFrame lastFrame;
            TLS3340VCOImpulseLUT *impulse;
            TLS3340VCOImpulseTrain zeroPhaseImpulses;//at the beginnig of a vco cycle
            TLS3340VCOImpulseTrain pwmImpulses;//for square pwm
            TLS3340VCOImpulseTrain suboscillatorImpulses;
            TLS3340VCOImpulseTrain sawImpulses;

            TLS3340NoiseSource noise;

            double vcoFrequencyHz;
            double sampleRateHzInternal;
            double sampleRateHzExternal;
            double sampleTimeS; //time per sample, 1/f
            double currentPhaseStep;
            double phase;

            double currentSumOfZeroPhaseImpulses, currentSumOfPWMImpulses,currentSumOfSawImpulses,currentSumOfSuboscImpulses;
            double sawtoothSlope;
            double leakyIntegratorFactor,leakyIntegratorFactorSaw;

            int suboscillatorCounter;
            bool squareSwitch,sawSwitch;
            double pwmCoefficient; //range: -0.5 to +0.5
            int suboscillatorMode;
            int noiseIndex;

            double lastPitch = 261.6256;

            //for sinc interpolation (todo: Move iterpolation to struct or class)
                const int sincZeroCrossings = 3; //increase for even better anti aliasing
                const int sincOversampling  = 10000;
                LabSeven::LS3340::TLS3340VCOSINCLUT *sincOversampled;
                //nbrInternalSamples is much larger than neccessary
                //so that sincZeroCrossings can be increased without problems
                const int nbrInternalSamples = 1000;
                TLS3340VCOFrame internalSamples[1000];
                int nbrInternalSamplesInUse = 0;
                int internalSamplesIndex = 0;

                //depend on external samplerate
                int    radius;
                double sampleStep;
                double stretch;
                int nbrSamplesRequired;

                //runtime stuff
                int internalSamplesPointer;
                double sincPosition;
                double samplePhase = 0.0;
                double currentSincCoeff;
                double triangleTemp = 0.0;


            inline void setSamplerateInternal(double sampleRateHz)
            {
                sampleRateHzInternal = sampleRateHz;
                this->sampleTimeS = 1.0/sampleRateHzInternal;
                currentPhaseStep = vcoFrequencyHz * sampleTimeS;
                sawtoothSlope = currentPhaseStep;//vcoFrequencyHz * sampleTimeS;
            }

        public:
            inline void setSamplerateExternal(double sampleRateHz)
            {
                sampleRateHzExternal = sampleRateHz;

                //recalulate frequency dependent sinc interpolation parameters
                sampleStep = sampleRateHzInternal/sampleRateHzExternal;
                stretch    = 0.8*sampleRateHzExternal/sampleRateHzInternal;

                if (sampleRateHz <= 48000)
                {
                    stretch    = 0.8*sampleRateHzExternal/sampleRateHzInternal;
                }
                else if (sampleRateHz <= 96000)
                {
                    stretch    = 0.6*sampleRateHzExternal/sampleRateHzInternal;
                }
                else
                {
                    stretch    = 0.5*sampleRateHzExternal/sampleRateHzInternal;
                }

                radius     = floor((double)sincZeroCrossings/stretch);
                nbrSamplesRequired = radius+1;
                nbrInternalSamplesInUse = 2*radius+1;
            }

            inline void setFrequency(double frequencyHz)
            {
                this->vcoFrequencyHz = frequencyHz;
                currentPhaseStep = vcoFrequencyHz * sampleTimeS;
                sawtoothSlope = currentPhaseStep;//vcoFrequencyHz * sampleTimeS;
            }

            inline void setPwmCoefficient(double pwmCoefficient)
            {
                if (pwmCoefficient > 0.4)
                    this->pwmCoefficient =  0.4;
                else if (pwmCoefficient <  -0.4)
                    this->pwmCoefficient = -0.4;
                else
                    this->pwmCoefficient = pwmCoefficient;
            }

            inline void setSuboscillatorMode(int mode)
            {
                if (mode == 0)
                    this->suboscillatorMode = 0;
                else if (mode == 1)
                    this->suboscillatorMode = 1;
                else if (mode == 2)
                    this->suboscillatorMode = 2;
                else
                    this->suboscillatorMode = 1;
            }

            //can output several samples at a time
            inline void getNextBlock(TLS3340VCOFrame *block, size_t nbrFramesInBlock)
            {
                for (size_t i=0; i<nbrFramesInBlock; i++)
                {
                    //manage phase and pulse creation
                    phase += currentPhaseStep;
                    if (phase > 1.0)
                    {
                        //wrap phase for periodicity
                        phase -= 1.0;

                        //add new positive impulse
                        zeroPhaseImpulses.addImpulse(phase,currentPhaseStep);

                        //inc and wrap suboscillatorCounter
                        suboscillatorCounter++;
                        if (suboscillatorCounter > 3) suboscillatorCounter = 0;

                        switch (suboscillatorMode)
                        {
                            case 0: switch (suboscillatorCounter)
                                    {
                                        case 0: case 2: suboscillatorImpulses.addImpulse(phase,currentPhaseStep, 1.0);break;
                                        case 1: case 3: suboscillatorImpulses.addImpulse(phase,currentPhaseStep,-1.0);break;
                                        default: suboscillatorCounter=suboscillatorCounter;
                                    }
                                    break;
                            case 1: switch (suboscillatorCounter)
                                    {
                                        case 0: suboscillatorImpulses.addImpulse(phase,currentPhaseStep, 1.0);break;
                                        case 2: suboscillatorImpulses.addImpulse(phase,currentPhaseStep,-1.0);break;
                                        default: suboscillatorCounter=suboscillatorCounter;
                                    }
                                    break;
                            case 2: switch (suboscillatorCounter)
                                    {
                                        //amplitude set to 1.255 on purpose
                                        case 0: suboscillatorImpulses.addImpulse(phase,currentPhaseStep,1.255); break;
                                        case 1: suboscillatorImpulses.addImpulse(phase,currentPhaseStep,-0.25); break;
                                        case 2: suboscillatorImpulses.addImpulse(phase,currentPhaseStep,-0.005); break;
                                        case 3:	suboscillatorImpulses.addImpulse(phase,currentPhaseStep,-1.0);break;
                                        default: suboscillatorCounter=suboscillatorCounter;
                                    }
                        }

                        //tell squareSwitch that a new period has begun
                        squareSwitch = true;
                        sawSwitch    = true;
                    }
                    else
                    {
                        //pwmCoefficient is in [-0.4,+0.4] für 10% to 90% pwm range
                        if (squareSwitch && phase > 0.5+pwmCoefficient)
                        {
                            //add new negative impulse
                            pwmImpulses.addImpulse(phase-(0.5+pwmCoefficient),currentPhaseStep,-1.0);
                            squareSwitch = false;
                        }

                        if (sawSwitch && phase > 0.5)
                        {
                            //add new negative impulse
                            sawImpulses.addImpulse(phase-0.5,currentPhaseStep,-1.0);
                            sawSwitch = false;
                        }
                    }

                    //get current sums of impulses
                    zeroPhaseImpulses.getNextSumOfImpulsesAndSawtoothSlope(impulse,currentSumOfZeroPhaseImpulses);
                    pwmImpulses.getNextSumOfImpulsesAndSawtoothSlope(impulse,currentSumOfPWMImpulses);
                    sawImpulses.getNextSumOfImpulsesAndSawtoothSlope(impulse, currentSumOfSawImpulses);
                    suboscillatorImpulses.getNextSumOfImpulsesAndSawtoothSlope(impulse,currentSumOfSuboscImpulses);

                    //put together the waveforms
                    leakyIntegratorFactor    = 0.9998;
                    leakyIntegratorFactorSaw = 0.9998 - 0.05*2.0*vcoFrequencyHz*sampleTimeS;

                    block[i].sawtooth = currentSumOfSawImpulses + sawtoothSlope + leakyIntegratorFactorSaw * lastFrame.sawtooth;
                    block[i].square   = currentSumOfZeroPhaseImpulses + currentSumOfPWMImpulses + leakyIntegratorFactor * lastFrame.square;
                    block[i].subosc   = currentSumOfSuboscImpulses + leakyIntegratorFactorSaw * lastFrame.subosc;
                    block[i].noise    = noise.getNextNoiseSample();

                    triangleTemp = currentSumOfZeroPhaseImpulses + currentSumOfSawImpulses + leakyIntegratorFactor * triangleTemp;
                    block[i].triangle = 4.9*sawtoothSlope*triangleTemp + leakyIntegratorFactorSaw * lastFrame.triangle;

                    lastFrame.sawtooth = block[i].sawtooth;
                    lastFrame.square   = block[i].square;
                    lastFrame.triangle = block[i].triangle;
                    lastFrame.subosc   = block[i].subosc;

                    lastPitch = vcoFrequencyHz;
                }
            }

            //output with sinc interpolation
            inline void getNextFrameAtExternalSampleRateSinc(TLS3340VCOFrame *f)
            {
                //get the necessary number of new internal samples
                for (int j = 0; j < nbrSamplesRequired; j++)
                {
                    getNextBlock(&(internalSamples[internalSamplesIndex]),1);

                    internalSamplesIndex++;
                    if (internalSamplesIndex >= nbrInternalSamples)
                        internalSamplesIndex -= nbrInternalSamples;
                }

                //calculate interpolation
                internalSamplesPointer = internalSamplesIndex;
                f->square   = 0.0;
                f->sawtooth = 0.0;
                f->subosc   = 0.0;
                f->noise    = 0.0;
                f->triangle = 0.0;
                sincPosition = ((double)(nbrInternalSamplesInUse-(radius+1))-samplePhase)*stretch;

                for (int j = 0; j < nbrInternalSamplesInUse; j++)
                {
                    currentSincCoeff = sincOversampled->getValAt(sincPosition);

                    f->square   += internalSamples[internalSamplesPointer].square * currentSincCoeff;
                    f->sawtooth += internalSamples[internalSamplesPointer].sawtooth * currentSincCoeff;
                    f->subosc   += internalSamples[internalSamplesPointer].subosc * currentSincCoeff;
                    f->noise    += internalSamples[internalSamplesPointer].noise * currentSincCoeff;
                    f->triangle += internalSamples[internalSamplesPointer].triangle * currentSincCoeff;

                    sincPosition -= stretch;

                    internalSamplesPointer--;
                    if (internalSamplesPointer < 0)
                        internalSamplesPointer += nbrInternalSamples;
                }

                //rescale
                f->square   *= stretch;
                f->sawtooth *= stretch;
                f->subosc   *= stretch;
                f->noise    *= stretch;
                f->triangle *= stretch;

                //prepare parameters for next interpolation
                samplePhase += sampleStep;
                nbrSamplesRequired = (int)floor(samplePhase);
                samplePhase -= (double)nbrSamplesRequired;
            }

            inline void getNextFrameAtExternalSampleRateCubic(TLS3340VCOFrame *f)
            {
                //get the necessary number of new internal samples
                for (int j = 0; j < nbrSamplesRequired; j++)
                {
                    getNextBlock(&(internalSamples[internalSamplesIndex]),1);

                    internalSamplesIndex++;
                    if (internalSamplesIndex >= nbrInternalSamples)
                        internalSamplesIndex -= nbrInternalSamples;
                }

                //calculate interpolation
                f->square   = 0.0;
                f->sawtooth = 0.0;
                f->subosc   = 0.0;
                f->noise    = 0.0;
                f->triangle = 0.0;

                //Next neighbour interpolation as a stub
                //TODO: Replace by cubic interpolation and add a 'quality: std/hgh' switch to GUI
                *f = internalSamples[internalSamplesIndex];

                //prepare parameters for next interpolation
                samplePhase += sampleStep;
                nbrSamplesRequired = (int)floor(samplePhase);
                samplePhase -= (double)nbrSamplesRequired;
            }

            TLS3340VCO()
            {
                //defaults
                setSamplerateInternal(192000); //192k covers the complete bandwidth of the original impulse. Do not change!
                setSamplerateExternal(44100); //to be changed/updated by host software
                setFrequency(261.6256);

                //generate fitted LS3340 impulse and sinc resampler lookup tables
                impulse = new TLS3340VCOImpulseLUT();
                sincOversampled = new LabSeven::LS3340::TLS3340VCOSINCLUT(sincZeroCrossings,sincOversampling);

                //some initializations
                lastFrame.sawtooth=-0.7;
                suboscillatorCounter = 0;
                squareSwitch = false;
                pwmCoefficient = 0;
                suboscillatorMode = 1;
                noiseIndex = 0;

                currentSumOfSuboscImpulses    = 0.0;
                currentSumOfSawImpulses       = 0.0;
                currentSumOfZeroPhaseImpulses = 0.0;
                currentSumOfPWMImpulses       = 0.0;
            }

            ~TLS3340VCO()
            {
                delete impulse;
                delete sincOversampled;
            }
        };
    };
};

#endif // LABSEVEN_3340_VCO_H
