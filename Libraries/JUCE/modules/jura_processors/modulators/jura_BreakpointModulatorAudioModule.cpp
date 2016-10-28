//-------------------------------------------------------------------------------------------------
// construction/destruction:

BreakpointModulatorAudioModule::BreakpointModulatorAudioModule(CriticalSection *newPlugInLock,                                                          
  RAPT::rsBreakpointModulator *newBreakpointModulatorToWrap)                                                              
  : AudioModule(newPlugInLock)
{
  jassert( newBreakpointModulatorToWrap != NULL ); // you must pass a valid object to the constructor

  wrappedBreakpointModulator = newBreakpointModulatorToWrap;
  moduleName = juce::String("BreakpointModulator");
  setActiveDirectory(getApplicationDirectory() + juce::String("/BreakpointModulatorPresets") );
  initializeAutomatableParameters();
}

//-------------------------------------------------------------------------------------------------
// automation:

void BreakpointModulatorAudioModule::parameterChanged(Parameter* parameterThatHasChanged)
{
  if( wrappedBreakpointModulator == NULL )
    return;

  double value = parameterThatHasChanged->getValue();
  switch( getIndexOfParameter(parameterThatHasChanged) )
  {
  case  0: wrappedBreakpointModulator->setTimeScale(     value); break;
  case  1: wrappedBreakpointModulator->setTimeScaleByKey(value); break;
  case  2: wrappedBreakpointModulator->setTimeScaleByVel(value); break;
  case  3: wrappedBreakpointModulator->setDepth(         value); break;
  case  4: wrappedBreakpointModulator->setDepthByKey(    value); break;
  case  5: wrappedBreakpointModulator->setDepthByVel(    value); break;
  } // end of switch( parameterIndex )  
  markStateAsDirty();
}

//-------------------------------------------------------------------------------------------------
// state saving and recall:

int stringToModBreakpointShapeIndex(const juce::String &shapeString)
{
  if(      shapeString == "Stairstep" )     return rsModBreakpoint::STAIRSTEP;
  else if( shapeString == "Linear" )        return rsModBreakpoint::LINEAR;
  else if( shapeString == "Smooth" )        return rsModBreakpoint::SMOOTH;
  else if( shapeString == "Analog" )        return rsModBreakpoint::ANALOG;
  else if( shapeString == "AntiAnalog" )    return rsModBreakpoint::GROWING;
  else if( shapeString == "Sigmoid" )       return rsModBreakpoint::SIGMOID;
  else if( shapeString == "Spikey" )        return rsModBreakpoint::SPIKEY;
  else if( shapeString == "Sine 1" )        return rsModBreakpoint::SINE_1;
  else if( shapeString == "Sine 2" )        return rsModBreakpoint::SINE_2;
  else                                      return rsModBreakpoint::LINEAR;
}

const juce::String modBreakpointShapeIndexToString(int shapeIndex)
{
  if(      shapeIndex == rsModBreakpoint::STAIRSTEP ) return "Stairstep";
  else if( shapeIndex == rsModBreakpoint::LINEAR )    return "Linear";
  else if( shapeIndex == rsModBreakpoint::SMOOTH )    return "Smooth";
  else if( shapeIndex == rsModBreakpoint::ANALOG )    return "Analog";
  else if( shapeIndex == rsModBreakpoint::GROWING )   return "AntiAnalog";
  else if( shapeIndex == rsModBreakpoint::SIGMOID )   return "Sigmoid";
  else if( shapeIndex == rsModBreakpoint::SPIKEY )    return "Spikey";
  else if( shapeIndex == rsModBreakpoint::SINE_1 )    return "Sine 1";
  else if( shapeIndex == rsModBreakpoint::SINE_2 )    return "Sine 2";
  else                                                return juce::String::empty;
}

XmlElement* BreakpointModulatorAudioModule::getStateAsXml(const juce::String& stateName,                                                         
  bool markAsClean)
{
  // store the inherited controller mappings:
  XmlElement *xmlState = AudioModule::getStateAsXml(stateName, markAsClean);

  //// store the parameters of the underlying core object:
  //if( wrappedBreakpointModulator != NULL )
  //  xmlState = breakpointModulatorStateToXml(wrappedBreakpointModulator, xmlState);

  double doubleValue = 0.0;
  int    intValue    = 0;
  bool   boolValue   = false;
  juce::String stringValue = juce::String::empty;

  // add some attributes:
  xmlState->setAttribute(juce::String("ScaleFactor"),      wrappedBreakpointModulator->getScaleFactor() );
  xmlState->setAttribute(juce::String("Offset"),           wrappedBreakpointModulator->getOffset() );
  xmlState->setAttribute(juce::String("TimeScale"),        wrappedBreakpointModulator->getTimeScale() );
  xmlState->setAttribute(juce::String("TimeScaleByKey"),   wrappedBreakpointModulator->getTimeScaleByKey() );
  xmlState->setAttribute(juce::String("TimeScaleByVel"),   wrappedBreakpointModulator->getTimeScaleByVel() );
  xmlState->setAttribute(juce::String("Depth"),            wrappedBreakpointModulator->getDepth() );
  xmlState->setAttribute(juce::String("DepthByKey"),       wrappedBreakpointModulator->getDepthByKey() );
  xmlState->setAttribute(juce::String("DepthByVel"),       wrappedBreakpointModulator->getDepthByVel() );
  xmlState->setAttribute(juce::String("SyncMode"),         wrappedBreakpointModulator->isInSyncMode());
  xmlState->setAttribute(juce::String("LoopStartIndex"),   wrappedBreakpointModulator->getLoopStartIndex() );
  xmlState->setAttribute(juce::String("LoopEndIndex"),     wrappedBreakpointModulator->getLoopEndIndex() );

  if( wrappedBreakpointModulator->getLoopMode() == rsBreakpointModulator::FORWARD_LOOP )
    xmlState->setAttribute(juce::String("LoopMode"), juce::String("Forward") );
  else
    xmlState->setAttribute(juce::String("LoopMode"), juce::String("Off") );

  // create an XmlElement for each breakpoint and add it as child-XmlElement:
  for(int p = 0; p <= wrappedBreakpointModulator->lastBreakpointIndex(); p++)
  {
    XmlElement* breakpointState = new XmlElement(juce::String("Breakpoint"));

    breakpointState->setAttribute(juce::String("Time"),        wrappedBreakpointModulator->getBreakpointTime(p));
    breakpointState->setAttribute(juce::String("Level"),       wrappedBreakpointModulator->getBreakpointLevel(p));
    breakpointState->setAttribute(juce::String("Shape"),       modBreakpointShapeIndexToString(wrappedBreakpointModulator->getBreakpointShape(p) ) );
    breakpointState->setAttribute(juce::String("ShapeAmount"), wrappedBreakpointModulator->getBreakpointShapeAmount(p));

    xmlState->addChildElement(breakpointState);

  } // end of for(int p=0; p<=modulator->lastBreakpointIndex(); p++)

  return xmlState;
}

void BreakpointModulatorAudioModule::setStateFromXml(const XmlElement& xmlState,                                                    
  const juce::String& stateName, bool markAsClean)
{
  // restore the inherited controller mappings:
  AudioModule::setStateFromXml(xmlState, stateName, markAsClean);

  //// restore the parameters of the underlying core object:
  //if( wrappedBreakpointModulator != NULL )
  //  breakpointModulatorStateFromXml(wrappedBreakpointModulator, xmlState);

  bool success = true;  // get rid

  rsBreakpointModulator *modulator = wrappedBreakpointModulator; // use a shorter name here...

  // restore the settings:
  modulator->setScaleFactor(xmlState.getDoubleAttribute(   "ScaleFactor",    1.0));
  modulator->setOffset(xmlState.getDoubleAttribute(        "Offset",         0.0));
  modulator->setTimeScale(xmlState.getDoubleAttribute(     "TimeScale",      1.0));
  modulator->setTimeScaleByKey(xmlState.getDoubleAttribute("TimeScaleByKey", 0.0));
  modulator->setTimeScaleByVel(xmlState.getDoubleAttribute("TimeScaleByVel", 0.0));
  modulator->setDepth(xmlState.getDoubleAttribute(         "Depth",          1.0));
  modulator->setDepthByKey(xmlState.getDoubleAttribute(    "DepthByKey",     0.0));
  modulator->setDepthByVel(xmlState.getDoubleAttribute(    "DepthByVel",     0.0));

  juce::String stringValue; // for temporary storage

  // check if there are at least two child elements corresponding to two breakpoints (this is the 
  // minimum allowed number):
  if( xmlState.getNumChildElements() < 2 )
    jassertfalse;

  // initialize our preview-BreakpointModulator (this will result in two breakpoints at times 0.0
  // and 1.0 with levels 0.0 both):
  modulator->initialize();

  double time, level, shapeAmount;
  int    shape = 0;
  // for temporary storage of retrieved values

  // retrieve all child elements with tag 'Breakpoint' and put them into an array:
  juce::Array<XmlElement*> breakpointElements;
  findChildElementsWithTagName(breakpointElements, xmlState, "Breakpoint");
  if( breakpointElements.size() < 2 )
    jassertfalse;

  // restore the data of the first breakpoint:
  XmlElement* breakpointState;
  breakpointState = breakpointElements[0];
  if( breakpointState->hasAttribute("Time")      &&
    breakpointState->hasAttribute(  "Level")     &&
    breakpointState->hasAttribute(  "Shape")     &&
    breakpointState->hasAttribute(  "ShapeAmount") )
  {
    level       = breakpointState->getDoubleAttribute("Level", 0.0);
    shapeAmount = breakpointState->getDoubleAttribute("ShapeAmount", 1.0);
    stringValue = breakpointState->getStringAttribute("Shape", juce::String::empty);
    shape       = stringToModBreakpointShapeIndex(stringValue);
    modulator->modifyBreakpoint(0, 0.0, level, shape, shapeAmount);
  }
  else
    jassertfalse;

  // restore the data of the last breakpoint:
  breakpointState = breakpointElements[breakpointElements.size()-1];
  if( breakpointState->hasAttribute("Time")        &&
    breakpointState->hasAttribute(  "Level")       &&
    breakpointState->hasAttribute(  "Shape")       &&
    breakpointState->hasAttribute(  "ShapeAmount") ) 
  {
    time        = breakpointState->getDoubleAttribute("Time",        1.0);
    level       = breakpointState->getDoubleAttribute("Level",       0.0);
    shapeAmount = breakpointState->getDoubleAttribute("ShapeAmount", 1.0);
    stringValue = breakpointState->getStringAttribute("Shape", juce::String::empty);
    shape       = stringToModBreakpointShapeIndex(stringValue);
    modulator->modifyBreakpoint(1, time, level, shape, shapeAmount);
  }
  else
    jassertfalse;

  // insert all the intermediate breakpoints with their data:
  int breakpointIndex = 1;
  for( breakpointIndex=1; breakpointIndex<=breakpointElements.size()-2; breakpointIndex++ )
  {
    breakpointState = breakpointElements[breakpointIndex];
    if( breakpointState->hasAttribute("Time")        &&
      breakpointState->hasAttribute(  "Level")       &&
      breakpointState->hasAttribute(  "Shape")       &&
      breakpointState->hasAttribute(  "ShapeAmount")     ) 
    {
      time        = breakpointState->getDoubleAttribute("Time", 1.0);
      level       = breakpointState->getDoubleAttribute("Level", 0.0);
      shapeAmount = breakpointState->getDoubleAttribute("ShapeAmount", 1.0);
      stringValue = breakpointState->getStringAttribute("Shape", juce::String::empty);
      shape       = stringToModBreakpointShapeIndex(stringValue);
      modulator->insertBreakpoint(time, level, shape, shapeAmount);
    }
    else
      jassertfalse;
  }

  // restore the loop-settings:
  if( xmlState.hasAttribute("LoopMode") )
  {
    stringValue = xmlState.getStringAttribute("LoopMode" );
    if( stringValue == "Off" )
      modulator->setLoopMode(false);
    else if( stringValue == "Forward" )
      modulator->setLoopMode(true);
  }
  else
    jassertfalse;

  int index = 0;
  if( xmlState.hasAttribute("LoopStartIndex") )
  {
    index = xmlState.getIntAttribute("LoopStartIndex" );
    modulator->setLoopStartIndex(index);
  }
  else
    jassertfalse;

  if( xmlState.hasAttribute("LoopEndIndex") )
  {
    index = xmlState.getIntAttribute("LoopEndIndex" );
    modulator->setLoopEndIndex(index);
  }
  else
    jassertfalse;

  // restore the sync-setting:
  if( xmlState.hasAttribute("SyncMode") )
  {
    modulator->setSyncMode(xmlState.getBoolAttribute("SyncMode", true));
  }
  else
    jassertfalse;
}

void BreakpointModulatorAudioModule::setStateToDefaults()
{  
  if( wrappedBreakpointModulator != NULL )
    wrappedBreakpointModulator->initialize();
}

//-------------------------------------------------------------------------------------------------
// internal functions:

void BreakpointModulatorAudioModule::initializeAutomatableParameters()
{
  // create the automatable parameters and add them to the list - note that the order of the adds
  // is important because in parameterChanged(), the index (position in the array) will be used to
  // identify which particlua parameter has changed.

  juce::Array<double> defaultValues;

  // this pointer will be used to temporarily store the addresses of the created Parameter-objects:
  AutomatableParameter* p;

  // #00
  p = new AutomatableParameter(plugInLock, "TimeScale", 0.0625, 16.0, 0.0, 1.0, 
    Parameter::EXPONENTIAL);
  defaultValues.add(1.0/16.0);
  defaultValues.add(1.0/12.0);
  defaultValues.add(1.0/8.0);
  defaultValues.add(1.0/6.0);
  defaultValues.add(1.0/4.0);
  defaultValues.add(1.0/3.0);
  defaultValues.add(1.0/2.0);
  defaultValues.add(3.0/4.0);
  defaultValues.add(1.0);
  defaultValues.add(3.0/2.0);
  defaultValues.add(2.0);
  defaultValues.add(3.0);
  defaultValues.add(4.0);
  defaultValues.add(6.0);
  defaultValues.add(8.0);
  defaultValues.add(12.0);
  defaultValues.add(16.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #01
  p = new AutomatableParameter(plugInLock, "TimeScaleByKey", -150.0, 150.0, 1.0, 0.0, 
    Parameter::LINEAR);
  defaultValues.clear();
  defaultValues.add(-100.0);
  defaultValues.add(0.0);
  defaultValues.add(100.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #02
  p = new AutomatableParameter(plugInLock, "TimeScaleByVel", -150.0, 150.0, 1.0, 0.0, 
    Parameter::LINEAR);
  defaultValues.clear();
  defaultValues.add(-100.0);
  defaultValues.add(0.0);
  defaultValues.add(100.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #03
  p = new AutomatableParameter(plugInLock, "Depth", 0.0, 4.0, 0.0, 1.0, Parameter::LINEAR);
  defaultValues.clear();
  defaultValues.add(1.0/16.0);
  defaultValues.add(1.0/12.0);
  defaultValues.add(1.0/8.0);
  defaultValues.add(1.0/6.0);
  defaultValues.add(1.0/4.0);
  defaultValues.add(1.0/3.0);
  defaultValues.add(1.0/2.0);
  defaultValues.add(3.0/4.0);
  defaultValues.add(1.0);
  defaultValues.add(3.0/2.0);
  defaultValues.add(2.0);
  defaultValues.add(3.0);
  defaultValues.add(4.0);
  defaultValues.add(6.0);
  defaultValues.add(8.0);
  defaultValues.add(12.0);
  defaultValues.add(16.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #04
  p = new AutomatableParameter(plugInLock, "DepthByKey", -150.0, 150.0, 1.0, 0.0, 
    Parameter::LINEAR);
  defaultValues.clear();
  defaultValues.add(-100.0);
  defaultValues.add(0.0);
  defaultValues.add(100.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #05
  p = new AutomatableParameter(plugInLock, "DepthByVel", -150.0, 150.0, 1.0, 0.0, 
    Parameter::LINEAR);
  defaultValues.clear();
  defaultValues.add(-100.0);
  defaultValues.add(0.0);
  defaultValues.add(100.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // make a call to setValue for each parameter in order to set up all the slave voices:
  for(int i=0; i < (int) observedParameters.size(); i++ )
    parameterChanged(observedParameters[i]);
}