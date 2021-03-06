#include "rosic_FilterTests.h"
using namespace rotes;

//#include "rosic/rosic.h"
#include "../Shared/Plotting/rosic_Plotter.h"
using namespace rosic;

template<class T>
void getImpulseResponse(T &module, double *h, int N)
{
  module.reset();
  h[0] = module.getSample(1.0);
  for(int n = 1; n < N; n++)
    h[n] = module.getSample(0.0);
}

template<class T>
void plotImpulseResponse(T &module, int N, double fs)
{
  double *t = new double[N];  // time axis
  double *h = new double[N];  // impulse response

  rosic::fillWithRangeLinear(t, N, 0.0, (N-1)/fs);
  getImpulseResponse(module, h, N);
  Plotter::plotData(N, t, h);

  delete[] t;
  delete[] h;
}

template<class T>
void writeImpulseResponseToFile(const char *path, T &module, int N, int fs, int numBits)
{  
  double *h = new double[N];  
  getImpulseResponse(module, h, N);

  rosic::normalize(h, N, 1.0);

  rosic::writeToMonoWaveFile(path, h, N, fs, numBits);
  delete[] h;
}



void rotes::testLadderFilter()
{
  double fs   = 44100;  // samplerate in Hz
  double fcHi = 4000;   // 1st cutoff frequency
  double fcLo = 250;    // 2nd cutoff frequency 
  double fSaw = 120;    // frequency of the input sawtooth - 120 seems to be a problem-freq




  rosic::LadderFilter ladder;
  ladder.setSampleRate(fs);
  ladder.setResonance(0.95);
  //ladder.setResonance(0.0);
  //ladder.setResonance(0.5);




  /*
  static const int N = 500;  
  double t[N];  // time axis
  double yLo[N], yHi[N], yHiLo[N], yLoHi[N];  
    // output signals for low and high cutoff and the two switches lo->hi, hi->lo

  rosic::fillWithIndex(t, N);
  int n;

  // low cutoff output without switch:
  ladder.reset();
  ladder.setCutoff(fcLo, true);
  ladder.getSampleTest(1.0);  // only to se up the internal states, output irrelevant
  for(n = 0; n < N; n++)
    yLo[n] = ladder.getSampleTest(0.0);

  // low cutoff output with switch from high cutoff:
  ladder.reset();
  ladder.setCutoff(fcHi, true);
  ladder.getSampleTest(1.0);   
  ladder.setCutoff(fcLo, true);
  for(n = 0; n < N; n++)
    yHiLo[n] = ladder.getSampleTest(0.0);

  Plotter::plotData(N, t, yLo, yHiLo);
  */




  
  static const int N = 6000;
  double t[N];  // time axis
  double x[N];  // input signal
  double y[N];  // output signal

  rosic::synthesizeWaveform(x, N, SAW, fSaw, fs);
  rosic::fillWithIndex(t, N);
  rosic::fillWithZeros(y, N);

  int n;
  ladder.setCutoff(fcHi, true);
  for(n = 0; n < N/3; n++)
    y[n] = ladder.getSampleTest(x[n]);
  ladder.setCutoff(fcLo, true);
  for(n = N/3; n < 2*N/3; n++)
    y[n] = ladder.getSampleTest(x[n]);
  ladder.setCutoff(fcHi, true);
  for(n = 2*N/3; n < N; n++)
    y[n] = ladder.getSampleTest(x[n]);

  Plotter::plotData(N, t, x, y);




  int dummy = 0;
}

void rotes::testModalFilter()
{
  // user parameters:
  static const int N = 20000;   // # samples to plot
  double fs  = 44100;  // samplerate in Hz
  double td  = 0.1;    // decay time constant in seconds
  double f   = 100;    // frequency in Hz
  double phs = 45;     // phase in degrees
  double A   = 1.5;    // amplitude as raw factor
  double pm  = 0.0;    // phase-modulation (for the 2nd implementation)


  ModalFilter mf;
  mf.setModalParameters(f, A, td, phs, fs);

  ModalFilter2 mf2;
  mf2.setModalParameters(f, A, td, phs, fs);
  mf2.setPhaseModulation(pm);





  double t[N], h[N], h2[N];
  fillWithIndex(t, N);
  h[0]  = mf.getSample(1.0);
  h2[0] = mf2.getSample(1.0);
  for(int n = 1; n < N; n++)
  {
    //mf.setModalParameters(f+0.5*f*sin(0.01*n), A, td, phs, fs);
    h[n]  = mf.getSample(0.0);

    //mf2.setModalParameters(f, A, td, phs+n, fs);
    //mf2.setModalParameters(f+0.5*f*sin(0.01*n), A, td, phs, fs);
    h2[n] = mf2.getSample(0.0);
  }

  Plotter::plotData(N, t, h, h2);
  //Plotter::plotData(N, t, h2);


  writeImpulseResponseToFile("d:\\TmpData\\ModalImpulseResponse.wav", mf2, (int)fs, (int)fs, 16);
}

void rotes::testModalFilterWithAttack()
{
  // user parameters:
  double td  = 0.100;  // asymptotic decay time constant in seconds
  double tp  = 0.050;  // time of peak occurence in seconds (must be < td)
  double f   = 440;    // frequency in Hz
  double dt  = +0.0;   // detune for the quickly decaying sinusoid in semitones (adds roughness)
  double phs = 90;     // phase in degrees
  double A   = 1.0;    // amplitude as raw factor
  double fs  = 44100;  // samplerate in Hz

  double df  = pitchOffsetToFreqFactor(dt);

  ModalFilterWithAttack mf;
  mf.setModalParameters(f, A, tp, td, phs, fs, df);
  plotImpulseResponse(mf, (int)fs, (int)fs);
  //writeImpulseResponseToFile("d:\\TmpData\\ModalImpulseResponse.wav", mf, fs, fs, 16);
}

void rotes::testBiquadPhasePlot()
{
  int i;
  static const int numBiquads = 10;
  static const int numBins    = 500;

  // allpass parameters:
  double fs = 44100.0;        // sample-rate
  double fc = 0.5*fs/PI;      // characteristic frequency (such that wc = 1)
  double  q = 1.0/sqrt(2.0);  // quality factor
  q = 5.0;

  // create biquad cascade coefficients:
  double b0[numBiquads], b1[numBiquads], b2[numBiquads], a1[numBiquads], a2[numBiquads];
  //rosic::BiquadDesigner::calculateFirstOrderLowpassCoeffs(b0[0], b1[0], b2[0], a1[0], a2[0], 1.0/fs, fc);
  //rosic::BiquadDesigner::calculateFirstOrderLowpassCoeffsBilinear(b0[0], b1[0], b2[0], a1[0], a2[0], 1.0/fs, fc);
  rosic::BiquadDesigner::calculateCookbookAllpassCoeffs(b0[0], b1[0], b2[0], a1[0], a2[0], 1.0/fs, fc, q);
  a1[0] = -a1[0]; // needed because of different sign conventions in BiquadDesigner and FilterAnalyzer -> fix this
  a2[0] = -a2[0];
  for(i = 1; i < numBiquads; i++)
  {
    b0[i] = b0[0];
    b1[i] = b1[0];
    b2[i] = b2[0];
    a1[i] = a1[0];
    a2[i] = a2[0];
  }

  // create normalized radian frequency axis:
  double w[numBins];
  fillWithRangeLinear(w, numBins, 0.0, PI);

  // obtain magnitude phase response:
  double m[numBins];
  double p[numBins];
  FilterAnalyzer::getBiquadCascadeMagnitudeResponse(b0, b1, b2, a1, a2, numBiquads, w, m, numBins, false, false);
  FilterAnalyzer::getBiquadCascadePhaseResponse(b0, b1, b2, a1, a2, numBiquads, w, p, numBins, false);

  // plot the (magnitude- and) phase response:
  Plotter::plotData(numBins, w, p);
  //Plotter::plotData(numBins, w, m, p);
}

void rotes::testFiniteImpulseResponseDesigner()
{
  static const int length = 201;
  double impulseResponse[length];

  //FiniteImpulseResponseDesigner designer;
  //designer.setMode(FiniteImpulseResponseDesigner::BANDREJECT);
  //designer.setFrequency(5000.0);
  //designer.getImpulseResponse(impulseResponse, length);
  //designer.spectralReversal(impulseResponse, length);

  // plot the impulse response:
  double indices[length];
  rosic::fillWithIndex(indices, length);
  //Plotter::plotData(length, indices, impulseResponse);

  // plot the magnitude response:
  static const int fftLength = 8192;
  double frequencies[fftLength];
  double magnitudes[fftLength];
  double magnitudes1[fftLength];
  double magnitudes2[fftLength];
  double magnitudes3[fftLength];
  double magnitudes4[fftLength];
  double magnitudes5[fftLength];
  double phases[fftLength];
  double sampleRate = 44100.0;
  fillWithIndex(frequencies, fftLength);
  scale(frequencies, frequencies, fftLength, sampleRate/fftLength);
  fftMagnitudesAndPhases(impulseResponse, length, magnitudes, phases, fftLength);
  //Plotter::plotData(fftLength/2, frequencies, magnitudes);
  //Plotter::plotData(fftLength/2, frequencies, phases);
 


  FiniteImpulseResponseFilter filter;
  filter.setMode(FiniteImpulseResponseDesigner::LOWPASS);
  filter.setImpulseResponseLength(length);
  filter.setWindowType(WindowDesigner::RECTANGULAR);
  filter.setFrequency(10000.0);

  filter.getMagnitudeResponse(frequencies, magnitudes1, fftLength, true, false);
  rosic::clipBuffer(magnitudes1, fftLength, -200.0, 10.0);

  filter.setWindowType(WindowDesigner::BLACKMAN);
  filter.getMagnitudeResponse(frequencies, magnitudes2, fftLength, true, false);
  rosic::clipBuffer(magnitudes2, fftLength, -200.0, 10.0);

  filter.setWindowType(WindowDesigner::HAMMING);
  filter.getMagnitudeResponse(frequencies, magnitudes3, fftLength, true, false);
  rosic::clipBuffer(magnitudes2, fftLength, -200.0, 10.0);

  filter.setWindowType(WindowDesigner::HANN);
  filter.getMagnitudeResponse(frequencies, magnitudes4, fftLength, true, false);
  rosic::clipBuffer(magnitudes2, fftLength, -200.0, 10.0);

  filter.setWindowType(WindowDesigner::COSINE_SQUARED);
  filter.getMagnitudeResponse(frequencies, magnitudes5, fftLength, true, false);
  rosic::clipBuffer(magnitudes2, fftLength, -200.0, 10.0);


  //Plotter::plotData(fftLength/2, frequencies, magnitudes1, magnitudes2, magnitudes3, magnitudes4, magnitudes5);
  //Plotter::plotData(fftLength/2, frequencies, magnitudes4, magnitudes5); // Hann vs cos^2
  //Plotter::plotData(fftLength/2, frequencies, magnitudes4, magnitudes2);   // Hann vs Blackman
  Plotter::plotData(fftLength/2, frequencies, magnitudes2);   // Blackman



  /*
  // create some noise, filter it and write it into a file:
  static const int testLength = 44100;
  double noise[testLength];
  double filteredNoise[testLength+length-1];
  for(int n=0; n<testLength; n++)
    noise[n] = rosic::random(-1.0, 1.0);
  rosic::convolve(noise, testLength, impulseResponse, length, filteredNoise);

  writeToStereoWaveFile("D:/TmpAudio/FilteredNoise.wav", noise, filteredNoise, testLength, 44100, 16);
  */



  // test cosine power window overlap:

  int dummy = 0;
}


void rotes::testConvolverPartitioned()
{
  static const int impulseLength  = 5000;
  static const int responseLength = 201;
  static const int resultLength   = impulseLength+responseLength-1;

  double impulse[impulseLength];
  double impulseResponse[responseLength];
  double resultTrue[resultLength];
  double result[resultLength];
  double indices[resultLength];
  rosic::fillWithIndex(indices,impulseLength);

  rosic::fillWithZeros(impulse, impulseLength);
  impulse[1000] = 1.0;

  //FiniteImpulseResponseDesigner designer;
  //designer.setMode(FiniteImpulseResponseDesigner::LOWPASS);
  //designer.getImpulseResponse(impulseResponse, responseLength);
  rosic::fillWithRangeLinear(impulseResponse, responseLength, 1.5, 1.0);

  rosic::convolve(impulse, impulseLength, impulseResponse, responseLength, resultTrue);


  ConvolverPartitioned convolver;
  convolver.setImpulseResponse(impulseResponse, responseLength);
  for(int n=0; n<resultLength; n++)
    result[n] = convolver.getSample(impulse[n]);

  double maxDiff = rosic::maxError(resultTrue, result, resultLength);


  // plot:
  //Plotter::plotData(impulseLength, indices, impulse);
  //Plotter::plotData(responseLength, indices, impulseResponse);
  //Plotter::plotData(resultLength, indices, resultTrue, result);




  int dummy = 0;
}

void rotes::testFiniteImpulseResponseFilter()
{
  FiniteImpulseResponseFilter filter;
  filter.setMode(FiniteImpulseResponseDesigner::BANDPASS);
  filter.setFrequency(5000.0);





  // create some noise, filter it and write it into a file:
  static const int testLength = 44100;
  double noise[testLength];
  double filteredNoise[testLength];
  for(int n=0; n<testLength; n++)
  {
    noise[n]         = rosic::random(-1.0, 1.0);
    filteredNoise[n] = filter.getSample(noise[n]);
  }

  //rosic::convolve(noise, testLength, impulseResponse, length, filteredNoise);


  //writeToStereoWaveFile("D:/TmpAudio/FilteredNoise.wav", noise, filteredNoise, testLength, 44100, 16);

  int dummy = 0;
}

  
void rotes::testFilterAnalyzer()
{
  static const int maxOrder = 20;
  double filterFrequency    = 5000;
  double sampleRate         = 44100.0;
  int prototypeOrder        = 3;

  // the filter-designer:
  rosic::InfiniteImpulseResponseDesigner designer;
  designer.setApproximationMethod(PrototypeDesigner::BUTTERWORTH);
  designer.setMode(InfiniteImpulseResponseDesigner::LOWPASS);
  designer.setFrequency(filterFrequency);
  designer.setSampleRate(sampleRate);
  designer.setPrototypeOrder(prototypeOrder);

  // biquad coeffs:
  static const int maxNumBiquads = maxOrder/2;   // maxOrder should be even - otherwise we need '+1'
  double b0[maxNumBiquads];
  double b1[maxNumBiquads];
  double b2[maxNumBiquads];
  double a1[maxNumBiquads];
  double a2[maxNumBiquads];
  fillWithValue(b0, maxNumBiquads, -100.0);
  fillWithValue(b1, maxNumBiquads, -100.0);
  fillWithValue(b2, maxNumBiquads, -100.0);
  fillWithValue(a1, maxNumBiquads, -100.0);
  fillWithValue(a2, maxNumBiquads, -100.0);

  int numBiquads = designer.getNumBiquadStages();
  designer.getBiquadCascadeCoefficients(b0, b1, b2, a1, a2);

  // frequencies and response-stuff:
  static const int numBins = 1000;
  double  frequencies[numBins];
  double  omegas[numBins];
  double  magnitudes[numBins];
  Complex H[numBins];
  for(int k=0; k<numBins; k++)
  {
    frequencies[k] = 0.5 * k * sampleRate / (numBins-1);
    omegas[k]      = 2.0 * PI * frequencies[k] / sampleRate;
  }

  //FilterAnalyzer::getBiquadCascadeMagnitudeResponse(b0, b1, b2, a1, a2, numBiquads, omegas, magnitudes, numBins, true);

  FilterAnalyzer::getBiquadCascadeFrequencyResponse(         b0, b1, b2, a1, a2, numBiquads, omegas, H, numBins);
  FilterAnalyzer::multiplyWithBiquadCascadeFrequencyResponse(b0, b1, b2, a1, a2, numBiquads, omegas, H, numBins);
  FilterAnalyzer::addWithBiquadCascadeFrequencyResponse(     b0, b1, b2, a1, a2, numBiquads, omegas, H, numBins);


  FilterAnalyzer::getMagnitudes( H, magnitudes, numBins);
  FilterAnalyzer::convertToDecibels(magnitudes, numBins);

  rosic::clipBuffer(magnitudes, numBins, -60.0, 20.0);


  Plotter::plotData(numBins, frequencies, magnitudes);

  int dummy = 0;
}

void rotes::testBiquadCascade()
{
  double sampleRate = 44100.0;

  InfiniteImpulseResponseDesigner designer;
  rosic::BiquadCascade            biquadCascade;

  designer.setApproximationMethod(PrototypeDesigner::BUTTERWORTH);
  designer.setMode(InfiniteImpulseResponseDesigner::BANDPASS);
  designer.setPrototypeOrder(5);
  designer.setSampleRate(sampleRate);
  designer.setFrequency(5000.0);
  designer.setBandwidth(1.0);

  biquadCascade.setNumStages(designer.getNumBiquadStages());
  designer.getBiquadCascadeCoefficients(biquadCascade.getAddressB0(), biquadCascade.getAddressB1(), biquadCascade.getAddressB2(), 
    biquadCascade.getAddressA1(), biquadCascade.getAddressA2() );
  //biquadCascade.turnIntoAllpass();

  // frequencies and response-stuff:
  static const int numBins = 1000;
  double  frequencies[numBins];
  double  omegas[numBins];
  double  magnitudes[numBins];
  double  phases[numBins];
  Complex H[numBins];
  for(int k=0; k<numBins; k++)
  {
    frequencies[k] = 0.5 * k * sampleRate / (numBins-1);
    omegas[k]      = 2.0 * PI * frequencies[k] / sampleRate;
  }

  biquadCascade.getFrequencyResponse(omegas, H, numBins);
  FilterAnalyzer::getMagnitudes(H, magnitudes, numBins);
  FilterAnalyzer::getPhases(    H, phases,     numBins);
  //FilterAnalyzer::convertToDecibels(magnitudes, numBins);

  biquadCascade.getMagnitudeResponse(omegas, magnitudes, numBins, true, false);
  biquadCascade.getMagnitudeResponse(omegas, magnitudes, numBins, true, true);


  Plotter::plotData(numBins, frequencies, magnitudes, phases);

  int dummy = 0;
}

/*
void rotes::testCrossover4Way()
{
  CrossOver4Way crossover;

  double sampleRate        = 44100.0;
  int    slope11           = 24;   // middlemost slope
  int    slope21           = 48;
  int    slope22           = 96;
  double lowCrossoverFreq  = 2000.0;
  double midCrossoverFreq  = 5000.0;
  double highCrossoverFreq = 10000.0;

  double lowClipValue      = -40.0;   // lower magnitude limit in dB for the plots
  crossover.setSampleRate(sampleRate);

  // frequencies and response-stuff:
  static const int numBins = 1000;
  double  frequencies[numBins];
  double  omegas[numBins];
  Complex H_LP_1_1[numBins];
  Complex H_HP_1_1[numBins];
  Complex H_LP_2_1[numBins];
  Complex H_HP_2_1[numBins];
  Complex H_LP_2_2[numBins];
  Complex H_HP_2_2[numBins];
  Complex H_Low[numBins];
  Complex H_LowMid[numBins];
  Complex H_HighMid[numBins];
  Complex H_High[numBins];
  Complex H_Sum[numBins];
  double  magnitudesLP_1_1[numBins];
  double  magnitudesHP_1_1[numBins];
  double  magnitudesLow[numBins];
  double  magnitudesLowMid[numBins];
  double  magnitudesHigh[numBins];
  double  magnitudesHighMid[numBins];
  double  magnitudesSum[numBins];
  for(int k=0; k<numBins; k++)
  {
    frequencies[k] = 0.5 * k * sampleRate / (numBins-1);
    omegas[k]      = 2.0 * PI * frequencies[k] / sampleRate;
  }

  //-----------------------------------------------------------------------------------------------
  // 2 ways:

  crossover.setBandActive(false, 1, 0);
  crossover.setBandActive(false, 1, 1);
  crossover.setCrossoverFrequency(lowCrossoverFreq,  1, 0);
  crossover.setCrossoverFrequency(highCrossoverFreq, 1, 1);
  crossover.setCrossoverFrequency(midCrossoverFreq,  0, 0);

  crossover.setSlope(slope11, 0, 0);
  crossover.setSlope(slope21, 1, 0);
  crossover.setSlope(slope22, 1, 1);

  // obtain lowpass frequency response:
  crossover.stage1.crossoverL.getLowpassFrequencyResponse(frequencies, H_LP_1_1, numBins, false);
  FilterAnalyzer::getMagnitudes(H_LP_1_1, magnitudesLP_1_1, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesLP_1_1, numBins);
  rosic::clipBuffer(magnitudesLP_1_1, numBins, lowClipValue, 10.0);

  // obtain highpass frequency response:
  crossover.stage1.crossoverL.getHighpassFrequencyResponse(frequencies, H_HP_1_1, numBins, false);
  FilterAnalyzer::getMagnitudes(H_HP_1_1, magnitudesHP_1_1, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesHP_1_1, numBins);
  rosic::clipBuffer(magnitudesHP_1_1, numBins, lowClipValue, 10.0);

  // obtain sum frequency response:
  rosic::add(H_LP_1_1, H_HP_1_1, H_Sum, numBins);
  FilterAnalyzer::getMagnitudes(H_Sum, magnitudesSum, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesSum, numBins);

  copyBuffer(magnitudesLP_1_1, magnitudesLow, numBins);
  copyBuffer(magnitudesHP_1_1, magnitudesHigh, numBins);
    
  //Plotter::plotData(numBins, frequencies, magnitudesLow, magnitudesHigh, magnitudesSum);

  //-----------------------------------------------------------------------------------------------
  // 3 ways (lower band split further):

  crossover.setBandActive(true, 1, 0);

  // obtain frequency response for low band:
  crossover.stage1.crossoverL.getLowpassFrequencyResponse(   frequencies, H_LP_1_1, numBins, false);
  crossover.stage2[0].crossoverL.getLowpassFrequencyResponse(frequencies, H_LP_2_1, numBins, false);
  rosic::multiply(H_LP_1_1, H_LP_2_1, H_Low, numBins);
  FilterAnalyzer::getMagnitudes(H_Low, magnitudesLow, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesLow, numBins);
  rosic::clipBuffer(magnitudesLow, numBins, lowClipValue, 10.0);

  // obtain frequency response for low-mid band:
  crossover.stage2[0].crossoverL.getHighpassFrequencyResponse(frequencies, H_HP_2_1, numBins, false);
  rosic::multiply(H_LP_1_1, H_HP_2_1, H_LowMid, numBins);
  FilterAnalyzer::getMagnitudes(H_LowMid, magnitudesLowMid, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesLowMid, numBins);
  rosic::clipBuffer(magnitudesLowMid, numBins, lowClipValue, 10.0);

  // obtain frequency response for high band:
  crossover.stage1.crossoverL.getHighpassFrequencyResponse(frequencies, H_HP_1_1, numBins, false);
  rosic::copyBuffer(H_HP_1_1, H_High, numBins);
  crossover.highBranchCompensationAllpass.getFrequencyResponse(omegas, H_High, numBins, FilterAnalyzer::MULTIPLICATIVE_ACCUMULATION);
  FilterAnalyzer::getMagnitudes(H_High, magnitudesHigh, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesHigh, numBins);
  rosic::clipBuffer(magnitudesHigh, numBins, lowClipValue, 10.0);

  // obtain the summed frequency response:
  rosic::add(H_Low, H_LowMid, H_Sum, numBins);
  rosic::add(H_Sum, H_High,   H_Sum, numBins);
  FilterAnalyzer::getMagnitudes(H_Sum, magnitudesSum, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesSum, numBins);

  //Plotter::plotData(numBins, frequencies, magnitudesLow, magnitudesLowMid, magnitudesHigh, magnitudesSum);

  //-----------------------------------------------------------------------------------------------
  // 3 ways (upper band split further):

  crossover.setBandActive(false, 1, 0);
  crossover.setBandActive(true,  1, 1);

  // obtain frequency response for low band:
  crossover.stage1.crossoverL.getLowpassFrequencyResponse(frequencies, H_LP_1_1, numBins, false);
  rosic::copyBuffer(H_LP_1_1, H_Low, numBins);
  crossover.lowBranchCompensationAllpass.getFrequencyResponse(omegas, H_Low, numBins, FilterAnalyzer::MULTIPLICATIVE_ACCUMULATION);
  FilterAnalyzer::getMagnitudes(H_Low, magnitudesLow, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesLow, numBins);
  rosic::clipBuffer(magnitudesLow, numBins, lowClipValue, 10.0);

  // obtain frequency response for high-mid band:
  crossover.stage1.crossoverL.getHighpassFrequencyResponse(  frequencies, H_HP_1_1, numBins, false);
  crossover.stage2[1].crossoverL.getLowpassFrequencyResponse(frequencies, H_LP_2_2, numBins, false);
  rosic::multiply(H_HP_1_1, H_LP_2_2, H_HighMid, numBins);
  FilterAnalyzer::getMagnitudes(H_HighMid, magnitudesHighMid, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesHighMid, numBins);
  rosic::clipBuffer(magnitudesHighMid, numBins, lowClipValue, 10.0);

  // obtain frequency response for high band:
  crossover.stage2[1].crossoverL.getHighpassFrequencyResponse(frequencies, H_HP_2_2, numBins, false);
  rosic::multiply(H_HP_1_1, H_HP_2_2, H_High, numBins);
  FilterAnalyzer::getMagnitudes(H_High, magnitudesHigh, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesHigh, numBins);
  rosic::clipBuffer(magnitudesHigh, numBins, lowClipValue, 10.0);

  // obtain the summed frequency response:
  rosic::add(H_Low, H_HighMid, H_Sum, numBins);
  rosic::add(H_Sum, H_High,    H_Sum, numBins);
  FilterAnalyzer::getMagnitudes(H_Sum, magnitudesSum, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesSum, numBins);

  //Plotter::plotData(numBins, frequencies, magnitudesLow, magnitudesHighMid, magnitudesHigh, magnitudesSum);

  //-----------------------------------------------------------------------------------------------
  // 4 ways (upper band split further):

  crossover.setBandActive(true, 1, 0);
  crossover.setBandActive(true, 1, 1);

  // obtain frequency response for low band:
  crossover.stage1.crossoverL.getLowpassFrequencyResponse(   frequencies, H_LP_1_1, numBins, false);
  crossover.stage2[0].crossoverL.getLowpassFrequencyResponse(frequencies, H_LP_2_1, numBins, false);
  rosic::multiply(H_LP_1_1, H_LP_2_1, H_Low, numBins);
  crossover.lowBranchCompensationAllpass.getFrequencyResponse(omegas, H_Low, numBins, FilterAnalyzer::MULTIPLICATIVE_ACCUMULATION);
  FilterAnalyzer::getMagnitudes(H_Low, magnitudesLow, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesLow, numBins);
  rosic::clipBuffer(magnitudesLow, numBins, lowClipValue, 10.0);

  // obtain frequency response for low-mid band:
  crossover.stage2[0].crossoverL.getHighpassFrequencyResponse(frequencies, H_HP_2_1, numBins, false);
  rosic::multiply(H_LP_1_1, H_HP_2_1, H_LowMid, numBins);
  crossover.lowBranchCompensationAllpass.getFrequencyResponse(omegas, H_LowMid, numBins, FilterAnalyzer::MULTIPLICATIVE_ACCUMULATION);
  FilterAnalyzer::getMagnitudes(H_LowMid, magnitudesLowMid, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesLowMid, numBins);
  rosic::clipBuffer(magnitudesLowMid, numBins, lowClipValue, 10.0);

  // obtain frequency response for high-mid band:
  crossover.stage1.crossoverL.getHighpassFrequencyResponse(  frequencies, H_HP_1_1, numBins, false);
  crossover.stage2[1].crossoverL.getLowpassFrequencyResponse(frequencies, H_LP_2_2, numBins, false);
  rosic::multiply(H_HP_1_1, H_LP_2_2, H_HighMid, numBins);
  crossover.highBranchCompensationAllpass.getFrequencyResponse(omegas, H_HighMid, numBins, FilterAnalyzer::MULTIPLICATIVE_ACCUMULATION);
  FilterAnalyzer::getMagnitudes(H_HighMid, magnitudesHighMid, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesHighMid, numBins);
  rosic::clipBuffer(magnitudesHighMid, numBins, lowClipValue, 10.0);

  // obtain frequency response for high band:
  crossover.stage2[1].crossoverL.getHighpassFrequencyResponse(frequencies, H_HP_2_2, numBins, false);
  rosic::multiply(H_HP_1_1, H_HP_2_2, H_High, numBins);
  crossover.highBranchCompensationAllpass.getFrequencyResponse(omegas, H_High, numBins, FilterAnalyzer::MULTIPLICATIVE_ACCUMULATION);
  FilterAnalyzer::getMagnitudes(H_High, magnitudesHigh, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesHigh, numBins);
  rosic::clipBuffer(magnitudesHigh, numBins, lowClipValue, 10.0);

  // obtain the summed frequency response:
  rosic::add(H_Low, H_LowMid,  H_Sum, numBins);
  rosic::add(H_Sum, H_HighMid, H_Sum, numBins);
  rosic::add(H_Sum, H_High,    H_Sum, numBins);
  FilterAnalyzer::getMagnitudes(H_Sum, magnitudesSum, numBins);
  FilterAnalyzer::convertToDecibels(magnitudesSum, numBins);

  Plotter::plotData(numBins, frequencies, magnitudesLow, magnitudesLowMid, magnitudesHighMid, magnitudesHigh, magnitudesSum);

  int dummy = 0;
}
*/


void rotes::testCrossover4Way2()
{
  static const int numBins = 4096;
  double sampleRate        = 44100.0;
  int    slope11           = 48;   // middlemost slope
  int    slope21           = 48;
  int    slope22           = 48;
  double lowCrossoverFreq  = 250.0;
  double midCrossoverFreq  = 1000.0;
  double highCrossoverFreq = 6000.0;
  double lowClipValue      = -40.0;   // lower magnitude limit in dB for the plots

  // set up the crossover:
  CrossOver4Way crossover;
  crossover.setSampleRate(sampleRate);
  crossover.setCrossoverFrequency(lowCrossoverFreq,  1, 0);
  crossover.setCrossoverFrequency(highCrossoverFreq, 1, 1);
  crossover.setCrossoverFrequency(midCrossoverFreq,  0, 0);
  crossover.setSlope(slope11, 0, 0);
  crossover.setSlope(slope21, 1, 0);
  crossover.setSlope(slope22, 1, 1);
  crossover.setBandActive(true, 1, 0);
  crossover.setBandActive(true, 1, 1);

  // impulse response variables:
  double indices[numBins];
  fillWithIndex(indices, numBins);
  float impulseResponsesFloat[8*numBins];
  fillWithZeros(impulseResponsesFloat, 8*numBins);
  impulseResponsesFloat[0]       = 1.0;  // 1st sample of left input channel
  impulseResponsesFloat[numBins] = 1.0;  // 1st sample of right input channel

  float **impulseResponsePointers = new float*[8];
  for(int i=0; i<8; i++)
    impulseResponsePointers[i] = &impulseResponsesFloat[i*numBins];

  // obtain the impulse-responses:
  //crossover.processBuffer(impulseResponsesFloat, numBins);  // !!! TODO: re-activate this and adapt data types
  crossover.processBuffer(impulseResponsePointers, numBins);
  double impulseResponses[8*numBins];
  convertBuffer(impulseResponsesFloat, impulseResponses, 8*numBins);
  double impulseResponseSum[numBins];
  for(int n=0; n<numBins; n++)
  {
    impulseResponseSum[n] =   impulseResponses[n] + impulseResponses[2*numBins+n] + impulseResponses[4*numBins+n] 
                            + impulseResponses[6*numBins+n];
  }

  // plot impulse responses:
  Plotter::plotData(numBins, indices, &impulseResponses[0], &impulseResponses[2*numBins], &impulseResponses[4*numBins], 
    &impulseResponses[6*numBins], impulseResponseSum);





  /*
  // frequency response variables:
  double  frequencies[numBins];
  double  omegas[numBins];
  double  magnitudesLow[numBins];
  double  magnitudesLowMid[numBins];
  double  magnitudesHigh[numBins];
  double  magnitudesHighMid[numBins];
  double  magnitudesSum[numBins];
  for(int k=0; k<numBins; k++)
  {
    frequencies[k] = 0.5 * k * sampleRate / (numBins-1);
    omegas[k]      = 2.0 * PI * frequencies[k] / sampleRate;
  }

  // compute and plot magnitude response of the sum:
  fftMagnitudesAndPhases(impulseResponseSum, numBins, magnitudesSum, NULL, numBins);
  scale(magnitudesSum, numBins, (double)numBins);
  Plotter::plotData(numBins, frequencies, magnitudesSum);
  */



  delete impulseResponsePointers;
  int dummy = 0;
}

void rotes::testSlopeFilter()
{
  double sampleRate = 44100.0;

  SlopeFilter filter;
  filter.setSampleRate(sampleRate);
  filter.setSlope(+6);


  // obtain impulse-response, convert to spectrum and plot the magnitude response:

  static const int length = 8192;
  double impulseResponse[length];
  impulseResponse[0] = filter.getSample(1.0);
  int n;
  for(n=1; n<length; n++)
    impulseResponse[n] = filter.getSample(0.0);



  double frequencies[length];
  double magnitudes[length];
  double decibels[length];

  fillWithIndex(frequencies, length);
  scale(frequencies, frequencies, length, sampleRate/length);

  fftMagnitudesAndPhases(impulseResponse, length, magnitudes, NULL, length);

  for(n=0; n<length; n++)
    decibels[n] = amp2dB(length*magnitudes[n]);

  Plotter::plotData(length/4, frequencies, decibels); 


  int dummy = 0;
}

void rotes::testPrototypeDesigner()
{
  /*
  static const int N = 3;
  double  coeffs[N+1];  
  Complex roots[N+5]; 
  fillWithZeros(coeffs, N+1);
  makeBesselPolynomial(coeffs, N);
  reverse(coeffs, N+1);
  findPolynomialRoots(coeffs, N, roots);
  */




  Complex poles[10];
  Complex zeros[10];
  PrototypeDesigner designer;
  designer.setApproximationMethod(PrototypeDesigner::BESSEL);
  designer.getPolesAndZeros(poles, zeros);

  int dummy = 0;
}

void rotes::testLowpassToLowshelf()
{
  // user parameters:
  static const int N = 5; // prototype filter order - above 7, it becomes weird for elliptics, Bessel works up to 25
  double Gp  = 0.8;       // passband ripple for elliptic lowpass prototype
  double Gs  = 0.2;       // stopband gain for elliptic lowpass prototype
  double G0  = 0.2;       // reference gain for shelving filter
  double G   = 1.3;       // gain for shelving filter

  // test:
  G0 = 0.2;
  G  = 1.4;
  //G0 = 1.4;
  //G  = 0.2;

  // analog unit cutoff lowpass prototype design:
  Complex z[N], p[N]; // arrays of lowpass prototype poles and zeros
  double  k;          // lowpass prototype filter gain
  //PrototypeDesigner::getBesselLowpassZerosPolesAndGain(z, p, &k, N);
  PrototypeDesigner::getEllipticLowpassZerosPolesAndGain(z, p, &k, N, Gp, Gs);
  //Plotter::plotAnalogMagnitudeResponse(z, p, k, N, 0.0, 3.0, 1000);

  // transform lowpass to low-shelving:
  Complex zs[N], ps[N]; // arrays of lowshelf prototype poles and zeros
  double  ks;           // lowshelf prototype filter gain
  PoleZeroMapper::sLowpassToLowshelf(z, p, &k, zs, ps, &ks, N, G0, G);
  Plotter::plotAnalogMagnitudeResponse(zs, ps, ks, N, 0.0, 3.0, 2000);
}


void rotes::testBesselPrototypeDesign()
{
  // user parameters:
  static const int N = 9;  //  Bessel works up to 25 before it gets funky
  double fc  = 1000.0;     // cutoff frequency
  double fl  =  500.0;     // lower bandedge frequency for bandpasses
  double fu  = 1500.0;     // upper bandedge frequency for bandpasses
  double G0  = 2.2;        // reference gain for shelving filter
  double G   = 1.4;        // gain for shelving filter

  // radian cutoff frequencies:
  double wc  = 2*PI*fc;   
  double wl  = 2*PI*fl;   
  double wu  = 2*PI*fu;   

  // todo: use pre-warped radian cutoff frequencies:
  double wcw = wc;        
  double wlw = wl;
  double wuw = wu;

  // just for test:
  wcw = 1000.0;           
  wlw =  500.0;
  wuw = 1500.0;

  // analog low-shelving design:
  Complex z[N], p[N]; // arrays of lowshelf prototype poles and zeros
  double  k;          // lowshelf prototype filter gain
  PrototypeDesigner::getBesselLowShelfZerosPolesAndGain(z, p, &k, N, G, G0);
  Plotter::plotAnalogMagnitudeResponse(z, p, k, N, 0.0, 3.0, 2000);
  //Plotter::plotAnalogPhaseResponse(z, p, k, N, 0.0, 3.0, 2000);


  Complex zp[N], pp[N]; // arrays of lowpass prototype poles and zeros
  double  kp;          // lowpass prototype filter gain
  PrototypeDesigner::getBesselLowpassZerosPolesAndGain(zp, pp, &kp, N);
  //Plotter::plotAnalogMagnitudeResponse(zp, pp, kp, N, 0.0, 3.0, 2000);
  //Plotter::plotAnalogPhaseResponse(zp, pp, kp, N, 0.0, 3.0, 2000);


  // s-plane frequency transformations:  

  Complex zLP[N], pLP[N], zHP[N], pHP[N], zBP[2*N], pBP[2*N], zBR[2*N], pBR[2*N];
  double  kLP, kHP, kBP, kBR; 

  // LP -> LP:
  PoleZeroMapper::sLowpassToLowpass(z, p, &k, zLP, pLP, &kLP, N, wcw);
  //Plotter::plotAnalogMagnitudeResponse(zLP, pLP, kLP, N, 0.0, 3.0*wcw, 200);

  // LP -> HP:
  PoleZeroMapper::sLowpassToHighpass(z, p, &k, zHP, pHP, &kHP, N, wcw);
  //Plotter::plotAnalogMagnitudeResponse(zHP, pHP, kHP, N, 0.0, 3.0*wcw, 200);
  //Plotter::plotAnalogPhaseResponse(zHP, pHP, kHP, N, 0.0, 3.0*wcw, 200);

  // LP -> BP:
  PoleZeroMapper::sLowpassToBandpass(z, p, &k, zBP, pBP, &kBP, N, wlw, wuw);
  //Plotter::plotAnalogMagnitudeResponse(zBP, pBP, kBP, 2*N, 0.0, 3.0*wcw, 200);

  // LP -> BR:
  PoleZeroMapper::sLowpassToBandreject(z, p, &k, zBR, pBR, &kBR, N, wlw, wuw);
  //Plotter::plotAnalogMagnitudeResponse(zBR, pBR, kBR, 2*N, 0.0, 3.0*wcw, 200);
}


void rotes::testPapoulisPrototypeDesign()
{
  // user parameters:
  static const int N = 6; // 
  double G0  = 0.2;        // reference gain for shelving filter
  double G   = 1.4;        // gain for shelving filter

  // with N = 12, G0 = 1, G >= 85 or so, the shelving numerator zeros come out wrong - there are repeated left-halfplane zeros
  // that shouldn't be there - ill numerical conditioning?
  G0 = 1.0;
  G  = 100.0;

  // analog low-shelving design:
  Complex z[N], p[N]; // arrays of lowshelf prototype poles and zeros
  double  k;          // lowshelf prototype filter gain
  //PrototypeDesigner::getPapoulisLowpassZerosPolesAndGain(z, p, &k, N);
  PrototypeDesigner::getPapoulisLowShelfZerosPolesAndGain(z, p, &k, N, G, G0);
  Plotter::plotAnalogMagnitudeResponse(z, p, k, N, 0.0, 3.0, 2000);
}

void rotes::testEngineersFilter()
{
  // filter parameters:
  double fs  = 44100.0;       // sample-rate
  double fc  = 500.0;         // characteristic frequency in Hz
  double bw  = 2.0;           // bandwidth in octaves (for bandpass, bandreject and bandstop)
  double g   = 2.0;           // gain for (peak and shelving filters)
  double rp  = 1.0;           // passband ripple in dB
  double rs  = 60.0;          // stopband rejection in dB
  int order  = 7;             // order of the prototype
  int mode   = 1;             // LP, HP, BP, BR, LS, HS, PK - todo: make allpass mode available
  int method = 5;             // approximation method

  // plotting parameters:
  static const int numSamples = 1000;
    
  // allocate and initialize memory for the responses:
  double timeAxis[numSamples];  // time axis in samples
  double stepResp[numSamples];  // step response
  double impResp[numSamples];   // impulse response
  fillWithRangeLinear(timeAxis, numSamples, 0.0, numSamples-1);

  // create and set up the filter:
  rsEngineersFilter filter;
  filter.setSampleRate(fs);
  filter.setFrequency(fc);
  filter.setBandwidth(bw);
  filter.setGain(g);
  filter.setRipple(rp);
  filter.setStopbandRejection(rs);
  filter.setPrototypeOrder(order);  
  filter.setMode(mode);
  filter.setApproximationMethod(method);

  // obtain and plot impulse- and step-response:
  int n;          // sample counter
  filter.reset(); // necesarry? -> shouldn't
  impResp[0] = filter.getSample(1.0);
  for(n = 1; n < numSamples; n++)
    impResp[n] = filter.getSample(0.0);
  filter.reset();
  for(n = 0; n < numSamples; n++)
    stepResp[n] = filter.getSample(1.0);
  Plotter::plotData(numSamples, timeAxis, impResp, stepResp);

  // todo: maybe plot magnitude, phase, phase-delay, group-delay, poles/zeros, etc. also
}

void rotes::testPoleZeroMapping()
{
  static const int N  = 10;      // prototype filter order
  double           fc = 5000.0;  // cutoff frequency in Hz
  double           fs = 44100.0; // samplerate in Hz

  Complex z[2*N], p[2*N];        // arrays of poles and zeros
  double  k = 1.0;               // filter gain


  InfiniteImpulseResponseDesigner iirDesigner;
  iirDesigner.setApproximationMethod(rosic::PrototypeDesigner::ELLIPTIC);
  iirDesigner.setMode(InfiniteImpulseResponseDesigner::LOWPASS);
  iirDesigner.setPrototypeOrder(N);
  iirDesigner.setSampleRate(fs);
  iirDesigner.setFrequency(fc);
  iirDesigner.setRipple(3.0);
  iirDesigner.setStopbandRejection(60.0);
  iirDesigner.getPolesAndZeros(p, z);

  //double c rosic::PoleZeroMapper::getAllpassWarpCoefficient(

  int dummy = 0;
}

void rotes::highOrderFilterPolesAndZeros()
{
  static const int N = 6;    // prototype filter order
  double fs    = 44100.0;  // samplerate
  double fc    =  1000.0;  // cutoff frequency
  double Ap    =     1.0;  // passband ripple in dB
  double As    =    50.0;  // stopband rejection in dB
  double bw    =     1.0;  // bandwidth in octaves
  int mode   = InfiniteImpulseResponseDesigner::BANDPASS;
  int method = PrototypeDesigner::ELLIPTIC;

  // create and set up the filter designer object:
  InfiniteImpulseResponseDesigner designer;
  designer.setPrototypeOrder(N);
  designer.setSampleRate(fs);
  designer.setFrequency(fc);
  designer.setRipple(Ap);
  designer.setStopbandRejection(As);
  designer.setBandwidth(bw);
  designer.setMode(mode);
  designer.setApproximationMethod(method);


  // compute poles and zeros:
  Complex z[2*N], p[2*N];        // arrays of poles and zeros
  designer.getPolesAndZeros(p, z);




  int dummy = 0;
}