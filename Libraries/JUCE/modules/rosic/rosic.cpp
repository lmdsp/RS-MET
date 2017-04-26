#ifdef ROSIC_H_INCLUDED
/* When you add this cpp file to your project, you mustn't include it in a file where you've
already included any other headers - just put it inside a file on its own, possibly with your config
flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
header files that the compiler may be using. */
#error "Incorrect use of JUCE cpp file"
#endif

#include "rosic.h"

/** The cpp files are included in the order in which they depend on each other. 
ToDo: reorder them in the library accordingly, such that files in one library folder depend only on 
other files in folders that are considered "above" in the hierarchy. 
In the future, rosic should be made dependent on the RAPT library and whereever it makes sense, the
rosic-class should be turned into an template instatiation of  RAPT class template.
jura_processors should depend on the rosic module and grab its DSP code from there
rename modules:
rosic: rs_dsp (this should never depend on any juce class/module)
jura_framework: rs_framework
jura_processors: rs_audio_processors
...namespace name should be rs (but do all of this only after dragging in all old plugin code and 
integrating it into the Chainer.

*/

// basics (but we needed to intersperse some stuff from other folders)
#include "basics/GlobalFunctions.cpp"
#include "basics/rosic_ChannelMatrix2x2.cpp"
#include "basics/rosic_Constants.cpp"                        // empty
#include "basics/rosic_FunctionTemplates.cpp"                // empty
#include "basics/rosic_HelperFunctions.cpp"
#include "basics/rosic_Interpolator.cpp"
#include "basics/rosic_NumberManipulations.cpp"              // empty
#include "infrastructure/rosic_MutexLock.cpp"                // used by SampleBuffer
#include "basics/rosic_SampleBuffer.cpp"
#include "basics/rosic_SamplePlaybackParameters.cpp"
#include "math/rosic_ElementaryFunctionsReal.cpp"            // used by SpecialFunctionsReal?
#include "math/rosic_RealFunctionEvaluationAlgorithms.cpp"   // used by SpecialFunctionsReal
#include "math/rosic_SpecialFunctionsReal.cpp"               // used by ComplexFunctions?
#include "math/rosic_Complex.cpp"                            // used by ComplexFunctionsAlgorithms
#include "math/rosic_ComplexFunctionsAlgorithms.cpp" 
#include "math/rosic_ComplexFunctions.cpp"                   // used by ExpressionEvaluator
#include "scripting/rosic_ExpressionEvaluator.cpp"           // used by TabulatedFunction
#include "basics/rosic_TabulatedFunction.cpp"                // needs ExpressionEvaluator
#include "basics/rosic_WarpedAllpassInterpolator.cpp"
#include "basics/rosic_WindowDesigner.cpp"

// datastructures
#include "datastructures/rosic_Array.cpp"                    // empty
#include "datastructures/rosic_ExtensionsForSTL.cpp"         // empty
#include "datastructures/rosic_KeyValueMap.cpp"              // empty
#include "datastructures/rosic_String.cpp"

// math (some of the cpp files in this folder are already included in the basics section):
#include "math/rosic_IntegerFunctions.cpp"
#include "math/rosic_LinearAlgebra.cpp"
#include "math/rosic_Vector.cpp"
#include "math/rosic_Matrix.cpp"
#include "math/rosic_MatrixVectorFunctions.cpp"
#include "math/rosic_Interpolation.cpp"
#include "math/rosic_PolynomialAlgorithms.cpp"
#include "math/rosic_PrimeNumbers.cpp"
#include "math/rosic_Transformations.cpp"

// filters
#include "filters/rosic_AllpassChain.cpp"
#include "filters/rosic_BiquadBase.cpp"
#include "filters/rosic_FilterAnalyzer.cpp"
#include "filters/rosic_BiquadCascade.cpp"
#include "filters/rosic_BiquadDesigner.cpp"
#include "filters/rosic_BiquadMonoDF1.cpp"
#include "filters/rosic_BiquadStereoDF1.cpp"


// analysis:
#include "analysis/rosic_CyclicAutoCorrelator.cpp"  // no dependencies
//#include "analysis/rosic_EnvelopeFollower.cpp"
//#include "analysis/rosic_FormantPreserver.cpp"
//#include "analysis/rosic_FormantRemover.cpp"
//#include "analysis/rosic_InstantaneousEnvelopeDetector.cpp"
//#include "analysis/rosic_LevelDetector.cpp"


