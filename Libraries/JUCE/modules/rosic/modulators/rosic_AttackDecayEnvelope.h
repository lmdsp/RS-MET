#ifndef rosic_AttackDecayEnvelope_h
#define rosic_AttackDecayEnvelope_h

//// rosic-indcludes:
//#include "../math/rosic_ElementaryFunctionsReal.h"

namespace rosic
{

  /**

  This is a class implements an envelope generator based on feeding the output signal of a 
  multiplicative accumulator (which is decaying exponential) into a leaky integrator (which 
  smoothes out the (otherwise instantaneous) attack). The output of the envelope is normalized to
  the range 0...1.

  */

  class AttackDecayEnvelope
  {

  public:

    //---------------------------------------------------------------------------------------------
    // construction/destruction:

    /** Constructor. */
    AttackDecayEnvelope();  

    /** Destructor. */
    ~AttackDecayEnvelope(); 

    //---------------------------------------------------------------------------------------------
    // parameter settings:

    /** Sets the sample-rate. */
    void setSampleRate(double newSampleRate);  

    /** Sets the time instant at which the peak excursion occurs (in milliseconds). */
    void setPeakTime(double newPeakTime);

    /** Sets the time constant for the multiplicative accumulator (which we consider as primarily 
    responsible for the decaying part) in milliseconds. */
    void setDecayTimeConstant(double newTimeConstant);

    //---------------------------------------------------------------------------------------------
    // inquiry:

    /** Returns the time instant at which the peak excursion occurs (in milliseconds). */
    double getPeakTime() const { return 1000.0*tp; }

    /** Returns the length of the decay phase (in milliseconds). */
    double getDecayTimeConstant() const { return 1000.0*td; }

    /** True, if output is below some threshold. */
    bool endIsReached(double threshold);  

    //---------------------------------------------------------------------------------------------
    // audio processing:

    /** Calculates one output sample at a time. */
    INLINE double getSample();    

    //---------------------------------------------------------------------------------------------
    // others:

    void trigger(bool startFromCurrentLevel);

    /** Resets the time variable. */
    void reset();   

  protected:

    /** Calculates the attack time constant ta from the desired time-instant of the peak excursion 
    tp and the decay time constant td. */
    void calculateAttackTimeConstant();

    /** Calculates the filter coefficients and the in initial value for the internal state of the
    decay-filter (this realizes the amplitude normalization). */
    void calculateCoeffsAndInitValue();

    double ca, cd;     // coefficients for attack and decay
    double ya, yd;     // previous outputs of attack and decay filter
    double ydInit;     // initial value for previous output of decay filter
    double ta, td;     // attack and decay time-constants (in seconds)
    double tp;         // time-instant of the peak excursion (in seconds);
    double fs;         // sample-rate
    int    n,  np;     // current sample-index and sample-index of the peak

  };

  //-----------------------------------------------------------------------------------------------
  // from here: definitions of the functions to be inlined, i.e. all functions which are supposed 
  // to be called at audio-rate (they can't be put into the .cpp file):

  INLINE double AttackDecayEnvelope::getSample()
  {
    yd *= cd;
    ya  = yd + ca*(ya-yd);
    return ya;
  }

} // end namespace rosic

#endif // rosic_AttackDecayEnvelope_h
