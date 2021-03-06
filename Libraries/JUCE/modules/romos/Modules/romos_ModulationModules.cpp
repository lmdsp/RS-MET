#include "romos_ModulationModules.h"
#include "../Algorithms/romos_Interpolation.h"

namespace romos
{




  void EnvelopeADSR::initialize()
  { 
    initInputPins( 7, rosic::rsString("Att"), rosic::rsString("AtSh"), rosic::rsString("Dec"), rosic::rsString("DcSh"), rosic::rsString("Sus"), 
                      rosic::rsString("Rel"), rosic::rsString("RlSh"));
    initOutputPins(1, rosic::rsString("Env"));

    // wrap this into a function setPinDefaultValue(int pinIndex, double newDefaultValue):
    //inputPins[4].setDefaultValue(1.0); // otherwise, the TimeScale pin is 0 when disconnected leading to an all zero output
    //int dummy = 0;
  }


  void EnvelopeADSR::processWithoutTriggerFlagCheck(Module *module, const double *in1, const double *in2, const double *in3, 
                                                    const double *in4, const double *in5, const double *in6, const double *in7, 
                                                    double *out, const int voiceIndex)
  { 
    EnvelopeADSR  *env     = static_cast<EnvelopeADSR*> (module);
    unsigned long *counter = &(env->counters[voiceIndex]);
    double         yStart  = env->startValues[voiceIndex];

    double sustain   = *in5;
    double timeScale = 1.0;
    unsigned long attackSamples  = (unsigned long) rosic::round(*in1 * timeScale * processingStatus.getSystemSampleRate()); 
    unsigned long decaySamples   = (unsigned long) rosic::round(*in3 * timeScale * processingStatus.getSystemSampleRate());
    unsigned long releaseSamples = (unsigned long) rosic::round(*in6 * timeScale * processingStatus.getSystemSampleRate()); 

      // todo: precompute timeScale * processingStatus.getSystemSampleRate() - or maybe get rid of timeScale

    double a;  
    double p;  // proportion of passedLength/fullLength of the phase ( = I(x), page 184, Eq.3-30)
    if( *counter < attackSamples )
    {
      // attack phase
      a = *in2;
      p = (double) *counter / (double) attackSamples;
      if( a == 0.0 )
        *out = yStart + (1.0 - yStart) * p;
      else
        *out = yStart + (1.0 - yStart) * (1.0 - exp(p*a)) / (1.0 - exp(a));
      *counter += 1;
    }
    else if( *counter < attackSamples + decaySamples )
    {
      // decay phase
      a = *in4;
      p = (double) (*counter-attackSamples)  / (double) (decaySamples);
      if( a == 0.0 )
        *out = 1.0 + (sustain-1.0) * p;
      else
        *out = 1.0 + (sustain-1.0) * (1.0 - exp(p*a)) / (1.0 - exp(a));
      *counter += 1;
    }
    else if( voiceAllocator.isNoteOn(voiceIndex) && *counter == attackSamples + decaySamples )
    {
      // the second part of the conditional is needed in order to not jump up to the sustain level when a new attack is started while
      // the envelope was in its release phase

      // sustain phase
      *out = sustain; 
    }
    else if( *counter < attackSamples + decaySamples + releaseSamples )
    {
      // release phase
      a = *in7;
      p = (double) (*counter-attackSamples-decaySamples)  / (double) (releaseSamples);    
      if( a == 0.0 )
        *out = yStart - yStart * p;
      else
        *out = yStart - yStart * (1.0 - exp(p*a)) / (1.0 - exp(a));
      *counter += 1;
    }
    else
    {
      // end is reached
      *out = 0.0;
    }
  }
  INLINE void EnvelopeADSR::process(Module *module, double *in1, double *in2, double *in3, double *in4, double *in5, double *in6, 
                                                    double *in7, double *out, int voiceIndex)
  { 
    EnvelopeADSR  *env     = static_cast<EnvelopeADSR*> (module);

    // in case a note-on/off was triggered at this sample, we compute a preliminary value and use this value as value to start from - this
    // makes sure to correctly handle double-attacks and early releases
    if( voiceAllocator.getNoteOffTriggerFlag(voiceIndex) == true ) 
    { 
      // we check note-off flag first to ensure that it attacks again when note-on and -off occur simultanously

      double timeScale = 1.0; 
      unsigned long attackSamples  = (unsigned long) ::round(*in1 * timeScale * processingStatus.getSystemSampleRate()); 
      unsigned long decaySamples   = (unsigned long) ::round(*in3 * timeScale * processingStatus.getSystemSampleRate());
      if( env->counters[voiceIndex] >= attackSamples ) 
        env->startValues[voiceIndex] = *in5; // set it temporarily to the sustain value

      processWithoutTriggerFlagCheck(module, in1, in2, in3, in4, in5, in6, in7, out, voiceIndex);
      env->startValues[voiceIndex] = *out;
      env->counters[voiceIndex]    = attackSamples + decaySamples;  
    }
    if( voiceAllocator.getNoteOnTriggerFlag(voiceIndex) == true )
    {  
      processWithoutTriggerFlagCheck(module, in1, in2, in3, in4, in5, in6, in7, out, voiceIndex);
      env->startValues[voiceIndex] = *out;
      env->counters[voiceIndex]    = 0;  
    }

    processWithoutTriggerFlagCheck(module, in1, in2, in3, in4, in5, in6, in7, out, voiceIndex);
  }
  void EnvelopeADSR::resetVoiceState(int voiceIndex)
  {
    ModuleAtomic::resetVoiceState(voiceIndex);
    counters[voiceIndex]    = 0;  
    startValues[voiceIndex] = 0.0;  
  }
  void EnvelopeADSR::allocateMemory()
  {
    ModuleAtomic::allocateMemory();
    counters    = new unsigned long[getNumVoices()];
    startValues = new double       [getNumVoices()];
  }
  void EnvelopeADSR::freeMemory()
  {
    ModuleAtomic::freeMemory();
    delete[] counters;
    counters = NULL;
    delete[] startValues;
    startValues = NULL;
  }
  CREATE_AND_ASSIGN_PROCESSING_FUNCTIONS_7(EnvelopeADSR); 




}
