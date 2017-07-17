#ifndef RAPT_SCOPESCREENSCANNER_H_INCLUDED
#define RAPT_SCOPESCREENSCANNER_H_INCLUDED

/** This is a class for generating the sawtooth-shaped waveform used for scanning over the screen
horizontally in an oscilloscope in 1D mode. It provides synchronization with the incoming waveform 
by using a zero crossing detector. */

template<class T>
class rsScopeScreenScanner
{

public:

  //---------------------------------------------------------------------------------------------
  // \name Construction/Destruction

  /** Constructor. */
  rsScopeScreenScanner();

  //---------------------------------------------------------------------------------------------
  // \name Setup:

  /** Sets the sample-rate. */
  void setSampleRate(T newSampleRate);

  /** Sets the frequency that shoould be used when we are not in sync mode. */
  void setScanFreqNoSync(T newFrequency);

  /** Switches synchronization of the sawtooth to the input signal on/off */
  void setSync(bool shouldSync);


  //---------------------------------------------------------------------------------------------
  // \name Audio Processing

  /** Generates one sawtooth output sample at the time. You must pass the input signal value that
  is used for the pitch analysis. The value is between 0 and 1. */
  inline T getSample(T in);

  //---------------------------------------------------------------------------------------------
  // \name Misc:

  /** Resets the internal state. */
  void reset();


protected:

  T sampleRate, scanFreq;
  T sawPhase, sawInc;  // sawtooth phase and increment
  bool sync;

  int samplesSinceReset, samplesSinceLastZero, zeroCrossingCount;

};

//-----------------------------------------------------------------------------------------------
// inlined functions:

template<class T>
inline T rsScopeScreenScanner<T>::getSample(T in)
{
  T result = sawPhase;
  sawPhase += sawInc;
  if(sawPhase > 1)
  {
    reset(); // maybe advance by (sawPhase-1) instead of resetting to zero
    //sawPhase -= 1;
  }
  return result;
}

#endif
