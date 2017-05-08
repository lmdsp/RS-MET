#include "romos_FunctionModules.h"

namespace romos
{

  void ClipperModule::initialize()
  { 
    initInputPins( 3, rosic::String("In"), rosic::String("Min"), rosic::String("Max"));
    initOutputPins(1, rosic::String("Out"));
  }
  INLINE void ClipperModule::process(Module *module, double *in1, double *in2, double *in3, double *out, int voiceIndex)
  {
    *out = clip(*in1, *in2, *in3);
  }
  CREATE_AND_ASSIGN_PROCESSING_FUNCTIONS_3(ClipperModule); 


  void SinCosModule::initialize()
  { 
    initInputPins( 1, rosic::String("In"));
    initOutputPins(2, rosic::String("Sin"), rosic::String("Cos"));
  }
  INLINE void SinCosModule::process(Module *module, double *in, double *out, int voiceIndex)
  {
    rosic::sinCos(2.0*PI*(*in), out, out+1);
  }
  CREATE_AND_ASSIGN_PROCESSING_FUNCTIONS_1(SinCosModule); 


}
