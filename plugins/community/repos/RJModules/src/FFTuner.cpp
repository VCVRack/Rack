// #include "RJModules.hpp"
// #include "dsp/samplerate.hpp"
// #include "dsp/ringbuffer.hpp"
// #include <iostream>
// #include <cmath>
// #include <math.h>
// #include <signal.h>
// #include "libfft.h"

// /* -- some basic parameters -- */
// #define SAMPLE_RATE (41000)
// #define FFT_SIZE (8192)
// #define FFT_EXP_SIZE (13)
// #define NUM_SECONDS (20)
// #define HISTORY_SIZE (1<<21)

// /* -- functions declared and used here -- */
// void buildHammingWindow( float *window, int size );
// void buildHanWindow( float *window, int size );
// void applyWindow( float *window, float *data, int size );
// //a must be of length 2, and b must be of length 3
// void computeSecondOrderLowPassParameters( float srate, float f, float *a, float *b );
// //mem must be of length 4.
// float processSecondOrderFilter( float x, float *mem, float *a, float *b );
// void signalHandler( int signum ) ;

// static bool running = true;
// static char * NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// void buildHammingWindow( float *window, int size )
// {
//    for( int i=0; i<size; ++i )
//       window[i] = .54 - .46 * cos( 2 * M_PI * i / (float) size );
// }
// void buildHanWindow( float *window, int size )
// {
//    for( int i=0; i<size; ++i )
//       window[i] = .5 * ( 1 - cos( 2 * M_PI * i / (size-1.0) ) );
// }
// void applyWindow( float *window, float *data, int size )
// {
//    for( int i=0; i<size; ++i )
//       data[i] *= window[i] ;
// }
// void computeSecondOrderLowPassParameters( float srate, float f, float *a, float *b )
// {
//    float a0;
//    float w0 = 2 * M_PI * f/srate;
//    float cosw0 = cos(w0);
//    float sinw0 = sin(w0);
//    //float alpha = sinw0/2;
//    float alpha = sinw0/2 * sqrt(2);

//    a0   = 1 + alpha;
//    a[0] = (-2*cosw0) / a0;
//    a[1] = (1 - alpha) / a0;
//    b[0] = ((1-cosw0)/2) / a0;
//    b[1] = ( 1-cosw0) / a0;
//    b[2] = b[0];
// }
// float processSecondOrderFilter( float x, float *mem, float *a, float *b )
// {
//     float ret = b[0] * x + b[1] * mem[0] + b[2] * mem[1]
//                          - a[0] * mem[2] - a[1] * mem[3] ;

//         mem[1] = mem[0];
//         mem[0] = x;
//         mem[3] = mem[2];
//         mem[2] = ret;

//         return ret;
// }

// struct FFTuner: Module {
//     enum ParamIds {
//         CH1_PARAM,
//         NUM_PARAMS
//     };
//     enum InputIds {
//         CH1_INPUT,
//         NUM_INPUTS
//     };
//     enum OutputIds {
//         CH1_OUTPUT,
//         NUM_OUTPUTS
//     };


//     DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
//     DoubleRingBuffer<float, 16> outBuffer;
//     SampleRateConverter<1> src;

//     FFTuner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
//     void step() override;
// };

// void FFTuner::step() {

//     float in = inputs[CH1_INPUT].value;
//     if (!historyBuffer.full()) {
//         historyBuffer.push(in);
//     }
//     // How many samples do we need consume to catch up?
//     float consume = index - historyBuffer.size();
//     if (outBuffer.empty()) {
//         double ratio = 1.0;
//         if (consume <= -16)
//             ratio = 0.5;
//         else if (consume >= 16)
//             ratio = 2.0;

//         int inFrames = mini(historyBuffer.size(), 16);
//         int outFrames = outBuffer.capacity();
//         src.setRatioSmooth(ratio);
//         src.process((const Frame<1>*)historyBuffer.startData(), &inFrames, (Frame<1>*)outBuffer.endData(), &outFrames);
//         historyBuffer.startIncr(inFrames);
//         outBuffer.endIncr(outFrames);
//     }

//     float wet = 0.0;
//     if (!outBuffer.empty()) {
//         wet = outBuffer.shift();
//     }

//    float a[2], b[3], mem1[4], mem2[4];
//    float data[FFT_SIZE];
//    float datai[FFT_SIZE];
//    float window[FFT_SIZE];
//    float freqTable[FFT_SIZE];
//    char * noteNameTable[FFT_SIZE];
//    float notePitchTable[FFT_SIZE];
//    void * fft = NULL;

//    // build the window, fft, etc
//     /*
//        buildHanWindow( window, 30 );
//        for( int i=0; i<30; ++i ) {
//           for( int j=0; j<window[i]*50; ++j )
//              printf( "*" );
//           printf("\n");
//        }
//        exit(0);
//     */
//    buildHanWindow( window, FFT_SIZE );
//    fft = initfft( FFT_EXP_SIZE );
//    computeSecondOrderLowPassParameters( SAMPLE_RATE, 330, a, b );
//    mem1[0] = 0; mem1[1] = 0; mem1[2] = 0; mem1[3] = 0;
//    mem2[0] = 0; mem2[1] = 0; mem2[2] = 0; mem2[3] = 0;

//    //freq/note tables
//    for( int i=0; i<FFT_SIZE; ++i ) {
//       freqTable[i] = ( SAMPLE_RATE * i ) / (float) ( FFT_SIZE );
//    }

//    for( int i=0; i<FFT_SIZE; ++i ) {
//       noteNameTable[i] = NULL;
//       notePitchTable[i] = -1;
//    }

//    for( int i=0; i<127; ++i ) {
//       float pitch = ( 440.0 / 32.0 ) * pow( 2, (i-9.0)/12.0 ) ;
//       if( pitch > SAMPLE_RATE / 2.0 )
//          break;
//       //find the closest frequency using brute force.
//       float min = 1000000000.0;
//       int index = -1;
//       for( int j=0; j<FFT_SIZE; ++j ) {
//          if( fabsf( freqTable[j]-pitch ) < min ) {
//              min = fabsf( freqTable[j]-pitch );
//              index = j;
//          }
//       }
//       noteNameTable[index] = NOTES[i%12];
//       notePitchTable[index] = pitch;
//       //printf( "%f %d %s\n", pitch, index, noteNameTable[index] );
//    }


//    while( running )
//    {
//       // read some data
//       // err = Pa_ReadStream( stream, data, FFT_SIZE );
//       // if( err ) goto error; //FIXME: we don't want to err on xrun

//       data *= outBuffer;

//       // low-pass
//       //for( int i=0; i<FFT_SIZE; ++i )
//       //   printf( "in %f\n", data[i] );
//       for( int j=0; j<FFT_SIZE; ++j ) {
//          data[j] = processSecondOrderFilter( data[j], mem1, a, b );
//          data[j] = processSecondOrderFilter( data[j], mem2, a, b );
//       }
//       // window
//       applyWindow( window, data, FFT_SIZE );

//       // do the fft
//       for( int j=0; j<FFT_SIZE; ++j )
//          datai[j] = 0;
//       applyfft( fft, data, datai, false );

//       //find the peak
//       float maxVal = -1;
//       int maxIndex = -1;
//       for( int j=0; j<FFT_SIZE/2; ++j ) {
//          float v = data[j] * data[j] + datai[j] * datai[j] ;
// /*
//          printf( "%d: ", j*SAMPLE_RATE/(2*FFT_SIZE) );
//          for( int i=0; i<sqrt(v)*100000000; ++i )
//             printf( "*" );
//          printf( "\n" );
// */
//          if( v > maxVal ) {
//             maxVal = v;
//             maxIndex = j;
//          }
//       }
//       float freq = freqTable[maxIndex];
//       //find the nearest note:
//       int nearestNoteDelta=0;
//       while( true ) {
//          if( nearestNoteDelta < maxIndex && noteNameTable[maxIndex-nearestNoteDelta] != NULL ) {
//             nearestNoteDelta = -nearestNoteDelta;
//             break;
//          } else if( nearestNoteDelta + maxIndex < FFT_SIZE && noteNameTable[maxIndex+nearestNoteDelta] != NULL ) {
//             break;
//          }
//          ++nearestNoteDelta;
//       }
//       char * nearestNoteName = noteNameTable[maxIndex+nearestNoteDelta];
//       float nearestNotePitch = notePitchTable[maxIndex+nearestNoteDelta];
//       float centsSharp = 1200 * log( freq / nearestNotePitch ) / log( 2.0 );

//       // now output the results:
//       printf("\033[2J\033[1;1H"); //clear screen, go to top left
//       fflush(stdout);

//       printf( "Tuner listening. Control-C to exit.\n" );
//       printf( "%f Hz, %d : %f\n", freq, maxIndex, maxVal*1000 );
//       printf( "Nearest Note: %s\n", nearestNoteName );
//       if( nearestNoteDelta != 0 ) {
//          if( centsSharp > 0 )
//             printf( "%f cents sharp.\n", centsSharp );
//          if( centsSharp < 0 )
//             printf( "%f cents flat.\n", -centsSharp );
//       } else {
//          printf( "in tune!\n" );
//       }
//       printf( "\n" );
//       int chars = 30;
//       if( nearestNoteDelta == 0 || centsSharp >= 0 ) {
//          for( int i=0; i<chars; ++i )
//             printf( " " );
//       } else {
//          for( int i=0; i<chars+centsSharp; ++i )
//             printf( " " );
//          for( int i=chars+centsSharp<0?0:chars+centsSharp; i<chars; ++i )
//             printf( "=" );
//       }
//       printf( " %2s ", nearestNoteName );
//       if( nearestNoteDelta != 0 )
//          for( int i=0; i<chars && i<centsSharp; ++i )
//            printf( "=" );
//       printf("\n");
//    }





//     // passthrough
//     outputs[CH1_OUTPUT].value = inputs[CH1_INPUT].value;
// }




// FFTunerWidget::FFTunerWidget(FFTuner *module) : ModuleWidget(module) {
//     box.size = Vec(15*10, 380);

//     {
//         SVGPanel *panel = new SVGPanel();
//         panel->box.size = box.size;
//         panel->setBackground(SVG::load(assetPlugin(plugin, "res/FFTuner.svg")));
//         addChild(panel);
//     }

//     addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
//     addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
//     addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
//     addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

//     addParam(ParamWidget::create<RoundBlackKnob>(Vec(57, 79), module, FFTuner::CH1_PARAM, 0.0, 1.0, 0.5));
//     addInput(Port::create<PJ301MPort>(Vec(22, 100), Port::INPUT, module, FFTuner::CH1_INPUT));
//     addOutput(Port::create<PJ301MPort>(Vec(110, 85), Port::OUTPUT, module, FFTuner::CH1_OUTPUT));
// }


