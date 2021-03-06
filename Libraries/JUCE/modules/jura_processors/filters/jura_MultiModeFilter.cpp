
//-------------------------------------------------------------------------------------------------
// construction/destruction:

MultiModeFilterAudioModule::MultiModeFilterAudioModule(CriticalSection *newPlugInLock, rosic::MultiModeFilter *newMultiModeFilterToWrap)
 : AudioModule(newPlugInLock)
{
  jassert( newMultiModeFilterToWrap != NULL ); // you must pass a valid rosic-object
  wrappedMultiModeFilter = newMultiModeFilterToWrap;
  moduleName = juce::String("MultiModeFilter");

  // initialize the current directory for preset loading and saving:
  setActiveDirectory(getApplicationDirectory() + juce::String("/MultiModeFilterPresets") );

  // create and initialize the automatable parameters:
  initializeAutomatableParameters();
}

//-------------------------------------------------------------------------------------------------
// automation:

void MultiModeFilterAudioModule::parameterChanged(Parameter* parameterThatHasChanged)
{
  if( wrappedMultiModeFilter == NULL )
    return;

  // depending on the index of the parameterThatHasChanged, we now set up the corresponding 
  // parameter in the underlying rosic object:
  double value = parameterThatHasChanged->getValue();
  switch( getIndexOfParameter(parameterThatHasChanged) )
  {
  case  0: wrappedMultiModeFilter->setFrequencyNominal(          value);     break;  
  case  1: wrappedMultiModeFilter->setFrequencyByKey  (          value);     break;
  case  2: wrappedMultiModeFilter->setFrequencyByVel  (          value);     break;
  case  3: wrappedMultiModeFilter->setResonance(                 value);     break;
  case  4: wrappedMultiModeFilter->setQ(                         value);     break;
  case  5: wrappedMultiModeFilter->setAllpassFreq(               value);     break; 
  case  6: wrappedMultiModeFilter->setMakeUp(                    value);     break; 
  case  7: wrappedMultiModeFilter->setDrive(                     value);     break;
  //case  8: wrappedMultiModeFilter->setDc(                        value);     break;
  case  9: wrappedMultiModeFilter->setGain(                      value);     break;
  case 10: wrappedMultiModeFilter->setMorph(                     value);     break;
  case 11: wrappedMultiModeFilter->setOrder(               (int) value);     break;

  default:
    {
      // do nothing
    }
  } // end of switch( parameterIndex )

  markStateAsDirty();
}

//-------------------------------------------------------------------------------------------------
// state saving and recall:

// temporary - move into get/setStateFrom/ToXml

int stringToFilterModeIndex(const juce::String &modeString)
{
  if( modeString == juce::String("Bypass") )  
    return rosic::MultiModeFilterParameters::BYPASS;
  else if( modeString == juce::String("Moogish Lowpass") )  
    return rosic::MultiModeFilterParameters::MOOGISH_LOWPASS;
  else if( modeString == juce::String("Lowpass 6 dB/oct") )  
    return rosic::MultiModeFilterParameters::LOWPASS_6;
  else if( modeString == juce::String("Lowpass 12 dB/oct") )  
    return rosic::MultiModeFilterParameters::LOWPASS_RBJ;
  else if( modeString == juce::String("Highpass 6 dB/oct") )  
    return rosic::MultiModeFilterParameters::HIGHPASS_6;
  else if( modeString == juce::String("Highpass 12 dB/oct") )  
    return rosic::MultiModeFilterParameters::HIGHPASS_RBJ;
  else if( modeString == juce::String("Bandpass 2*6 dB/oct") )  
    return rosic::MultiModeFilterParameters::BANDPASS_RBJ;
  else if( modeString == juce::String("Bandstop 2*6 dB/oct") )  
    return rosic::MultiModeFilterParameters::BANDREJECT_RBJ;
  else if( modeString == juce::String("Peak/Dip") )  
    return rosic::MultiModeFilterParameters::PEAK_OR_DIP_RBJ;
  else if( modeString == juce::String("Low Shelv 1st order") )  
    return rosic::MultiModeFilterParameters::LOW_SHELV_1ST;
  else if( modeString == juce::String("Low Shelv 2nd order") )  
    return rosic::MultiModeFilterParameters::LOW_SHELV_RBJ;
  else if( modeString == juce::String("High Shelv 1st order") )  
    return rosic::MultiModeFilterParameters::HIGH_SHELV_1ST;
  else if( modeString == juce::String("High Shelv 2nd order") )  
    return rosic::MultiModeFilterParameters::HIGH_SHELV_RBJ;
  else if( modeString == juce::String("Allpass 1st order") )  
    return rosic::MultiModeFilterParameters::ALLPASS_1ST;
  else if( modeString == juce::String("Allpass 2nd order") )  
    return rosic::MultiModeFilterParameters::ALLPASS_RBJ;

  else if( modeString == juce::String("Morph Low/Band/High") )  
    return rosic::MultiModeFilterParameters::MORPH_LP_BP_HP;

  else if( modeString == juce::String("Morph Low/Peak/High") )  
    return rosic::MultiModeFilterParameters::MORPH_LP_PK_HP;

  // some more else ifs to come...

  else                                                 
    return rosic::MultiModeFilterParameters::BYPASS;
}

const juce::String filterModeIndexToString(int modeIndex)
{
  if(modeIndex == rosic::MultiModeFilterParameters::BYPASS ) 
    return juce::String("Bypass");
  else if( modeIndex == rosic::MultiModeFilterParameters::MOOGISH_LOWPASS ) 
    return juce::String("Moogish Lowpass");

  else if( modeIndex == rosic::MultiModeFilterParameters::LOWPASS_6 ) 
    return juce::String("Lowpass 6 dB/oct");
  else if( modeIndex == rosic::MultiModeFilterParameters::LOWPASS_RBJ ) 
    return juce::String("Lowpass 12 dB/oct");
  else if( modeIndex == rosic::MultiModeFilterParameters::HIGHPASS_6 ) 
    return juce::String("Highpass 6 dB/oct");
  else if( modeIndex == rosic::MultiModeFilterParameters::HIGHPASS_RBJ ) 
    return juce::String("Highpass 12 dB/oct");
  else if( modeIndex == rosic::MultiModeFilterParameters::BANDPASS_RBJ ) 
    return juce::String("Bandpass 2*6 dB/oct");
  else if( modeIndex == rosic::MultiModeFilterParameters::BANDREJECT_RBJ ) 
    return juce::String("Bandstop 2*6 dB/oct");
  else if( modeIndex == rosic::MultiModeFilterParameters::PEAK_OR_DIP_RBJ ) 
    return juce::String("Peak/Dip");
  else if( modeIndex == rosic::MultiModeFilterParameters::LOW_SHELV_1ST ) 
    return juce::String("Low Shelv 1st order");
  else if( modeIndex == rosic::MultiModeFilterParameters::LOW_SHELV_RBJ ) 
    return juce::String("Low Shelv 2nd order");
  else if( modeIndex == rosic::MultiModeFilterParameters::HIGH_SHELV_1ST ) 
    return juce::String("High Shelv 1st order");
  else if( modeIndex == rosic::MultiModeFilterParameters::HIGH_SHELV_RBJ ) 
    return juce::String("High Shelv 2nd order");
  else if( modeIndex == rosic::MultiModeFilterParameters::ALLPASS_1ST ) 
    return juce::String("Allpass 1st order");
  else if( modeIndex == rosic::MultiModeFilterParameters::ALLPASS_RBJ ) 
    return juce::String("Allpass 2nd order");

  else if(modeIndex == rosic::MultiModeFilterParameters::MORPH_LP_BP_HP ) 
    return juce::String("Morph Low/Band/High");

  else if(modeIndex == rosic::MultiModeFilterParameters::MORPH_LP_PK_HP ) 
    return juce::String("Morph Low/Peak/High");

  // some more else ifs to come...

  else                                              
    return juce::String("Bypass");
}

XmlElement* multiModeFilterStateToXml(MultiModeFilter* filter, 
  XmlElement* xmlElementToStartFrom)
{
  // the XmlElement which stores all the releveant state-information:
  XmlElement* xmlState;
  if( xmlElementToStartFrom == NULL )
    xmlState = new XmlElement(juce::String("MultiModeFilterState")); 
  else
    xmlState = xmlElementToStartFrom;

  // convert the filter mode into a string and store the mode-string in the XmlElement:
  juce::String modeString = filterModeIndexToString(filter->getMode());
  xmlState->setAttribute("Mode", modeString);

  // now store the filter parameters - depending on the chosen mode, the set of parameters to be 
  // stored may differ. we distiguish the different cases by means of the modeString:
  if( filter->getMode() == MultiModeFilterParameters::BYPASS )
  {
    // no more parameters to store, when filter is in bypass mode
  }
  else
  {
    // store the cutoff/center frequency parameters and it's key and velocity dependence:
    xmlState->setAttribute("Frequency",      filter->getFrequencyNominal()              );
    xmlState->setAttribute("FrequencyByKey", filter->getFrequencyByKey()                );
    xmlState->setAttribute("FrequencyByVel", filter->getFrequencyByVel()                );
  }

  if( filter->getMode() == MultiModeFilterParameters::MOOGISH_LOWPASS )
  {
    // store the parameters which are specific to the Moogish Lowpass mode:
    xmlState->setAttribute("Resonance",      filter->getResonance()                     );
    xmlState->setAttribute("Drive",          filter->getDrive()                         );
    xmlState->setAttribute("Order",          filter->getOrder()                         );
    xmlState->setAttribute("PreAllpass",     filter->getAllpassFreq()                   );
    xmlState->setAttribute("MakeUp",         filter->getMakeUp()                        );
  }
  if( filter->currentModeSupportsGain() )
    xmlState->setAttribute("Gain",           filter->getGain()                          );
  if( filter->currentModeSupportsQ() )
    xmlState->setAttribute("Q",              filter->getQ()                             );
  if( filter->currentModeSupportsTwoStages() )
    xmlState->setAttribute("TwoStages",      filter->usesTwoStages()                    );

  if( filter->getMode() == MultiModeFilterParameters::MORPH_LP_PK_HP )
    xmlState->setAttribute("Morph",              filter->getMorph()                     );

  return xmlState;
}

bool multiModeFilterStateFromXml(MultiModeFilter* filter, const XmlElement &xmlState)
{
  bool success = true;

  // restore the settings from the XmlElement:
  int modeIndex = stringToFilterModeIndex( xmlState.getStringAttribute("Mode", "Bypass") );
  filter->setMode( modeIndex );
  filter->setFrequencyNominal(xmlState.getDoubleAttribute("Frequency",      1000.0) );
  filter->setFrequencyByKey(  xmlState.getDoubleAttribute("FrequencyByKey",    0.0) );
  filter->setFrequencyByVel(  xmlState.getDoubleAttribute("FrequencyByVel",    0.0) );
  filter->setResonance(       xmlState.getDoubleAttribute("Resonance",         0.0) );
  filter->setQ(               xmlState.getDoubleAttribute("Q",           sqrt(0.5)) );
  filter->setGain(            xmlState.getDoubleAttribute("Gain",              0.0) );
  filter->setMorph(           xmlState.getDoubleAttribute("Morph",             0.0) );
  filter->setDrive(           xmlState.getDoubleAttribute("Drive",             0.0) );
  filter->useTwoStages(       xmlState.getBoolAttribute  ("TwoStages",       false) );
  filter->setOrder(           xmlState.getIntAttribute   ("Order",               4) );
  filter->setAllpassFreq(     xmlState.getDoubleAttribute("PreAllpass",    20000.0) );
  filter->setMakeUp(          xmlState.getDoubleAttribute("MakeUp",          100.0) );

  return success = true;
}

XmlElement* MultiModeFilterAudioModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
  // store the inherited controller mappings:
  XmlElement *xmlState = AudioModule::getStateAsXml(stateName, markAsClean);

  // store the parameters of the underlying core object:
  if( wrappedMultiModeFilter != NULL )
    xmlState = multiModeFilterStateToXml(wrappedMultiModeFilter, xmlState);

  return xmlState;
}

void MultiModeFilterAudioModule::setStateFromXml(const XmlElement& xmlState, 
                                                 const juce::String& stateName, bool markAsClean)
{
  // restore the inherited controller mappings:
  AudioModule::setStateFromXml(xmlState, stateName, false);
  //AudioModule::setStateFromXml(xmlState, stateName, markAsClean);

  // restore the parameters of the underlying core object:
  if( wrappedMultiModeFilter != NULL )
    multiModeFilterStateFromXml(wrappedMultiModeFilter, xmlState);

  if( markAsClean == true )
    markStateAsClean();
}

//-------------------------------------------------------------------------------------------------
// internal functions:

void MultiModeFilterAudioModule::initializeAutomatableParameters()
{
  // create the automatable parameters and add them to the list - note that the order of the adds
  // is important because in parameterChanged(), the index (position in the array) will be used to
  // identify which particlua parameter has changed.

  // WARNING: if you change some slider's name here, make sure to also change it in  the editor's
  // calls to getSliderByName - otherwise the editor wil dereference a NULL pointer

  std::vector<double> defaultValues;

  // this pointer will be used to temporarily store the addresses of the created 
  // AutomatableParameter-objects:
  AutomatableParameter* p;

  // #00
  p = new AutomatableParameter(lock, "Frequency", 20.0, 20000.0, 0.0, 1000.0, Parameter::EXPONENTIAL, 74);
  defaultValues.push_back(20.0);
  defaultValues.push_back(200.0);
  defaultValues.push_back(2000.0);
  defaultValues.push_back(20000.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);
    // todo: define more meaningful default values here - for example tune the frequency to 
    // harmonics (assuming keytrack==100%)

  // #01
  p = new AutomatableParameter(lock, "FrequencyByKey", -200.0, 200.0, 0.1, 0.0, Parameter::LINEAR);
  defaultValues.clear(); 
  defaultValues.push_back(0.0);
  defaultValues.push_back(25.0);
  defaultValues.push_back(50.0);
  defaultValues.push_back(75.0);
  defaultValues.push_back(100.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #02
  p = new AutomatableParameter(lock, "FrequencyByVel", -200.0, 200.0, 0.1, 0.0,  Parameter::LINEAR);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #03
  p = new AutomatableParameter(lock, "Resonance", 0.0, 100.0, 0.1, 10.0, Parameter::LINEAR, 71);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #04
  p = new AutomatableParameter(lock, "Q", 0.5, 50.0, 0.001, sqrt(0.5), Parameter::EXPONENTIAL, 71);
  defaultValues.clear(); 
  defaultValues.push_back(0.5);
  defaultValues.push_back(sqrt(0.5));
  defaultValues.push_back(1.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #05
  p = new AutomatableParameter(lock, "PreAllpass", 20.0, 20000.0, 0.0, 20000.0, Parameter::EXPONENTIAL);
  defaultValues.clear(); 
  defaultValues.push_back(20.0);
  defaultValues.push_back(200.0);
  defaultValues.push_back(2000.0);
  defaultValues.push_back(20000.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #06
  p = new AutomatableParameter(lock, "MakeUp", 0.0, 100.0, 1.0, 0.0,  Parameter::LINEAR);
  defaultValues.clear(); 
  defaultValues.push_back(0.0);
  defaultValues.push_back(25.0);
  defaultValues.push_back(50.0);
  defaultValues.push_back(75.0);
  defaultValues.push_back(100.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #07
  p = new AutomatableParameter(lock, "Drive", -24.0, 24.0, 0.01, 0.0, Parameter::LINEAR);
  defaultValues.clear(); 
  defaultValues.push_back(-24.0);
  defaultValues.push_back(-18.0);
  defaultValues.push_back(-12.0);
  defaultValues.push_back(-6.0);
  defaultValues.push_back(-3.0);
  defaultValues.push_back(0.0);
  defaultValues.push_back(3.0);
  defaultValues.push_back(6.0);
  defaultValues.push_back(12.0);
  defaultValues.push_back(18.0);
  defaultValues.push_back(24.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #08
  p = new AutomatableParameter(lock, "Dc", -1.0, 1.0, 0.01, 0.0, Parameter::LINEAR);
  addObservedParameter(p);

  // #09
  p = new AutomatableParameter(lock, "Gain", -60.0, 30.0, 0.01, 0.0, Parameter::LINEAR);
  defaultValues.clear(); 
  defaultValues.push_back(-24.0);
  defaultValues.push_back(-18.0);
  defaultValues.push_back(-12.0);
  defaultValues.push_back(-6.0);
  defaultValues.push_back(-3.0);
  defaultValues.push_back(0.0);
  defaultValues.push_back(3.0);
  defaultValues.push_back(6.0);
  defaultValues.push_back(12.0);
  defaultValues.push_back(18.0);
  defaultValues.push_back(24.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #10
  p = new AutomatableParameter(lock, "Morph", -0.99, 0.99, 0.01, 0.0, Parameter::LINEAR);
  defaultValues.clear(); 
  defaultValues.push_back(-0.99);
  defaultValues.push_back(0.5);
  defaultValues.push_back(0.99);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  // #11
  p = new AutomatableParameter(lock, "Order", 0.0, 4.0, 1.0, 4.0, Parameter::LINEAR);
  addObservedParameter(p);

  // make a call to setValue for each parameter in order to set up all the slave voices:
  for(int i=0; i < (int) parameters.size(); i++ )
    parameterChanged(parameters[i]);
}

//=================================================================================================


MultiModeFreqResponseEditor::MultiModeFreqResponseEditor(const juce::String& name) 
  : SpectrumDisplayOld(name)
{
  setDescription("Drag around the node to adjust the filter's frequency and resonance, Q or gain");

  ParameterObserver::isGuiElement = true;
  filterToEdit = NULL;

  // set up the plot range:
  setMaximumRange(15.625, 32000.0, -60.0, 30.0);
  setCurrentRange(15.625, 32000.0, -60.0, 30.0);
  setHorizontalCoarseGrid(12.0, false);
  setHorizontalFineGrid(   3.0, false);
  setVerticalCoarseGridVisible( false);
  setVerticalFineGridVisible(   false);

  plotColourScheme.setCurveColouringStrategy(PlotColourScheme::UNIFORM);

  currentMouseCursor = MouseCursor(MouseCursor::NormalCursor);
  setMouseCursor(currentMouseCursor);

  dotRadius = 5.f;

  freqParameter  = NULL;
  resoParameter  = NULL;
  qParameter     = NULL;
  gainParameter  = NULL;
  morphParameter = NULL;

  // this stuff will be (re-) allocated in resized():
  numBins     = 0;
  frequencies = NULL;
  magnitudes  = NULL;

  // activate automation for this ParameterObserver:
  ParameterObserver::localAutomationSwitch = true;
}

MultiModeFreqResponseEditor::~MultiModeFreqResponseEditor(void)
{
  // remove ourselves as listeners from the Parameter objects, such that they do not try to notify a nonexistent listener:
  ParameterObserver::localAutomationSwitch = false;
  if( freqParameter != NULL )
    freqParameter->deRegisterParameterObserver(this);
  if( resoParameter != NULL )
    resoParameter->deRegisterParameterObserver(this);
  if( qParameter != NULL )
    qParameter->deRegisterParameterObserver(this);
  if( gainParameter != NULL )
    gainParameter->deRegisterParameterObserver(this);
  if( morphParameter != NULL )
    morphParameter->deRegisterParameterObserver(this);

  deleteAndZero(frequencies);
  deleteAndZero(magnitudes);
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// parameter-settings:

void MultiModeFreqResponseEditor::setFilterToEdit(rosic::MultiModeFilter* newFilterToEdit)
{
  filterToEdit = newFilterToEdit;
}

void MultiModeFreqResponseEditor::assignParameterFreq(Parameter *parameterToAssign)
{
  freqParameter = parameterToAssign;
  if( freqParameter != NULL )
    freqParameter->registerParameterObserver(this);
}

void MultiModeFreqResponseEditor::assignParameterReso(Parameter *parameterToAssign)
{
  resoParameter = parameterToAssign;
  if( resoParameter != NULL )
    resoParameter->registerParameterObserver(this);
}

void MultiModeFreqResponseEditor::assignParameterQ(Parameter *parameterToAssign)
{
  qParameter = parameterToAssign;
  if( qParameter != NULL )
    qParameter->registerParameterObserver(this);
}

void MultiModeFreqResponseEditor::assignParameterGain(Parameter *parameterToAssign)
{
  gainParameter = parameterToAssign;
  if( gainParameter != NULL )
    gainParameter->registerParameterObserver(this);
}

void MultiModeFreqResponseEditor::assignParameterMorph(Parameter *parameterToAssign)
{
  morphParameter = parameterToAssign;
  if( morphParameter != NULL )
    morphParameter->registerParameterObserver(this);
}

void MultiModeFreqResponseEditor::unAssignParameterFreq()
{
  if( freqParameter != NULL )
    freqParameter->deRegisterParameterObserver(this);
  freqParameter = NULL;
}

void MultiModeFreqResponseEditor::unAssignParameterReso()
{
  if( resoParameter != NULL )
    resoParameter->deRegisterParameterObserver(this);
  resoParameter = NULL;
}

void MultiModeFreqResponseEditor::unAssignParameterQ()
{
  if( qParameter != NULL )
    qParameter->deRegisterParameterObserver(this);
  qParameter = NULL;
}

void MultiModeFreqResponseEditor::unAssignParameterGain()
{
  if( gainParameter != NULL )
    gainParameter->deRegisterParameterObserver(this);
  gainParameter = NULL;
}

void MultiModeFreqResponseEditor::unAssignParameterMorph()
{
  if( morphParameter != NULL )
    morphParameter->deRegisterParameterObserver(this);
  morphParameter = NULL;
}

void MultiModeFreqResponseEditor::parameterChanged(Parameter* parameterThatHasChanged)
{
  sendChangeMessage();
  updatePlot();
}

void MultiModeFreqResponseEditor::parameterIsGoingToBeDeleted(Parameter* parameterThatWillBeDeleted)
{

  // clear reference to parameter that will be deleted

}

void MultiModeFreqResponseEditor::updateWidgetFromAssignedParameter(bool sendMessage)
{
  updatePlot();
  if( sendMessage == true )
    sendChangeMessage();
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// callbacks:

void MultiModeFreqResponseEditor::changeListenerCallback(ChangeBroadcaster *objectThatHasChanged)
{
  // temporarily switch the wantsAutomationNotification flag from the ParameterObserver base 
  // class off to avoid circular notifications and updates:
  localAutomationSwitch = false;

  // call the method which updates the widget:
  updatePlot();
  //updateWidgetFromAssignedParameter(false);


  // switch the wantsAutomationNotification flag on again:  
  localAutomationSwitch = true;
}

void MultiModeFreqResponseEditor::mouseDown(const MouseEvent &e)
{

  if( filterToEdit == NULL )
    return;

  // preliminray: do not open the MIDI-learn menu on right-button - show the image export menu 
  // instead (inherited behaviour from CoordinateSytem):
  if( e.mods.isRightButtonDown() )
    CoordinateSystemOld::mouseDown(e);
  else
  {
    // get the position of the event in components coordinates
    double x = e.getMouseDownX();
    double y = e.getMouseDownY();

    setupFilterAccordingToMousePosition(x, y);

    // send out a change-message:
    sendChangeMessage();
  }

  /*
  if( e.mods.isRightButtonDown() && xParameter != NULL && yParameter != NULL )
  {
  // prepare some strings for the popup menu:
  int freqCC = xParameter->getAssignedMidiController();
  juce::String freqjuce::String;
  if( freqCC > -1 )
  freqjuce::String = juce::String(T("(currently CC#")) + juce::String(freqCC) + juce::String(T(")"));
  else
  freqjuce::String = juce::String(T("(currently none)")); 
  juce::String minFreqjuce::String = hertzToStringWithUnitTotal5(xParameter->getLowerAutomationLimit());
  juce::String maxFreqjuce::String = hertzToStringWithUnitTotal5(xParameter->getUpperAutomationLimit());

  int resoCC = yParameter->getAssignedMidiController();
  juce::String resojuce::String;
  if( resoCC > -1 )
  resojuce::String = juce::String(T("(currently CC#")) + juce::String(resoCC) + juce::String(T(")"));
  else
  resojuce::String = juce::String(T("(currently none)")); 

  // ToDo: different cases - y can be reso, q or gain
  juce::String minResojuce::String = percentToStringWithUnit1(yParameter->getLowerAutomationLimit());
  juce::String maxResojuce::String = percentToStringWithUnit1(yParameter->getUpperAutomationLimit());

  // create a context menu to allow for MIDI learn and setting up min and max automation values:
  PopupMenu menu;
  menu.addItem(1, juce::String(T("MIDI learn frequency ") + freqjuce::String)  );
  menu.addItem(2, juce::String(T("MIDI learn resonance ") + resojuce::String)  );
  menu.addItem(3, juce::String(T("use current values as lower limits"))  );
  menu.addItem(4, juce::String(T("use current values as upper limits"))  );
  menu.addItem(5, juce::String(T("revert to defaults"))                  );

  const int result = menu.show();

  // retrieve current characteristic frequency and resonance:
  double freq = filterToEdit->getFrequencyNominal(); // frequency
  double reso = filterToEdit->getResonance();        // resonance

  if (result == 0)
  {
  // user dismissed the menu without picking anything - do nothing
  }
  else if (result == 1)
  {
  // user picked the frequency learn item:
  xParameter->switchIntoMidiLearnMode();
  }
  else if (result == 2)
  {
  // user picked the resonance learn item:
  yParameter->switchIntoMidiLearnMode();
  }
  else if (result == 3)
  {
  // user picked the lower-limit item:
  xParameter->setLowerAutomationLimit(freq);
  yParameter->setLowerAutomationLimit(reso);
  }
  else if (result == 4)
  {
  // user picked the upper-limit item:
  xParameter->setUpperAutomationLimit(freq);
  yParameter->setUpperAutomationLimit(reso);
  }
  else if (result == 5)
  {
  // user picked the revert to defaults item:
  xParameter->revertToDefaults();
  yParameter->revertToDefaults();
  }
  } // end of  if( e.mods.isRightButtonDown() )
  */
}

void MultiModeFreqResponseEditor::mouseDrag(const juce::MouseEvent &e)
{
  if( filterToEdit == NULL )
    return;

  /*
  if( e.mods.isRightButtonDown() && xParameter != NULL && yParameter != NULL )
  {
  // ignore mouse drags whne the right button is down and we have assigned automatable 
  // parameters because in that case, the right click is used for opening the MIDI-learn popup
  }
  else...
  */

  // get the position of the event in components coordinates
  double x = e.getMouseDownX() + e.getDistanceFromDragStartX();
  double y = e.getMouseDownY() + e.getDistanceFromDragStartY();

  setupFilterAccordingToMousePosition(x, y);

  // send out a change-message:
  sendChangeMessage();
}

void MultiModeFreqResponseEditor::setupFilterAccordingToMousePosition(double mouseX, 
  double mouseY)
{
  if( filterToEdit == NULL )
    return;

  // get the position of the event in components coordinates
  double x = mouseX;
  double y = mouseY;

  // convert them into a frequency and resonance/q/gain value:
  double gain = y;
  transformFromComponentsCoordinates(x, gain);
  gain = clip(gain, -60.0, 30.0);
  double freq = x;
  double reso = yToReso(y);
  double q    = yToQ(y);

  // restrict ranges (ToDo: actually the filter should take care of the itself....):
  freq = clip(freq, 20.0, 20000.0);
  reso = clip(reso, 0.0, 100.0);
  q    = clip(q, 0.5, 50.0);

  // set up the filter and raise automation events to update other widgets that represent the
  // parameters:
  filterToEdit->setFrequencyNominal(freq);
  if( freqParameter != NULL )
    freqParameter->setValue(freq, true, true);
  if( filterToEdit->getMode() == MultiModeFilterParameters::MOOGISH_LOWPASS )
  {
    filterToEdit->setResonance(reso);
    if( resoParameter != NULL )
      resoParameter->setValue(reso, true, true);
  }
  else if( filterToEdit->currentModeSupportsGain() )
  {
    filterToEdit->setGain(gain);
    if( gainParameter != NULL )
      gainParameter->setValue(gain, true, true);
  }
  else
  {
    filterToEdit->setQ(q);
    if( qParameter != NULL )
      qParameter->setValue(q, true, true);
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// drawing:

void MultiModeFreqResponseEditor::resized()
{
  SpectrumDisplayOld::resized();

  // (re) allocate and fill the arrays for the magnitude plot
  numBins = getWidth();
  if( frequencies == NULL )
    delete[] frequencies;
  if( magnitudes == NULL )
    delete[] magnitudes;
  frequencies = new double[numBins];
  magnitudes  = new double[numBins];
  getDisplayedFrequencies(frequencies, numBins);
  for(int i=0; i<numBins; i++)
    magnitudes[i]  = 0.0;
  updatePlot();
}

void MultiModeFreqResponseEditor::updatePlot()
{
  if( filterToEdit == NULL )
    return;

  // fill the magnitude array with the magnitudes:
  filterToEdit->setFrequencyInstantaneous(filterToEdit->getFrequencyNominal(), true);
  filterToEdit->getMagnitudeResponse(frequencies, magnitudes, numBins, true, false);
  clipBuffer(magnitudes, numBins, -120.0, 120.0);

  // overwrite the magnitude value at the bin closest to the cutoff-frequency with the magnitude at 
  // the exact cutoff frequency:
  double freq  = filterToEdit->getFrequencyNominal();
  double level = amp2dBWithCheck(filterToEdit->getMagnitudeAt(freq), 0.000001);
  level        = clip(level, -120.0, +120.0);

  for(int bin=0; bin < (numBins-1); bin++)
  {
    if( frequencies[bin] < freq && frequencies[bin+1] > freq )
    {
      if( fabs(frequencies[bin]-freq) <= fabs(frequencies[bin+1]-freq) ) // lower bin is closer
        magnitudes[bin]   = level;
      else                                                               // upper bin is closer
        magnitudes[bin+1] = level;
    }
  }

  setSpectrum(numBins, frequencies, magnitudes);
  //repaint();
}

void MultiModeFreqResponseEditor::plotCurveFamily(Graphics &g, juce::Image* targetImage, 
  XmlElement *targetSVG)
{
  if( filterToEdit == NULL )
    return;

  CurveFamilyPlotOld::plotCurveFamily(g, targetImage, targetSVG);


  //Colour graphColour = colourScheme.curves; // preliminary
  //if( colourScheme.plotColours.size() > 0 )
  //  graphColour = colourScheme.plotColours[0];
  Colour graphColour = plotColourScheme.getCurveColour(0);  
  //g.setColour(graphColour); 

  //double freq = filterToEdit->getFrequencyNominal(); // frequency
  //double reso = filterToEdit->getResonance();        // resonance

  // retrieve characteristic frequency and gain in order to draw the handle (y = gain will be used 
  // only if the mode dictates that - otherwise it will serve as dummy and the actual y-value will
  // be calculated seperately:
  double x = filterToEdit->getFrequencyNominal();
  double y = filterToEdit->getGain();  

  // determine the coordinates of the handle into image component coordinates (for export) or 
  // components coordinates for plot:
  if( targetImage == NULL )
    transformToComponentsCoordinates(x, y);
  else
    transformToImageCoordinates(x, y, targetImage);

  // y is now the gain in component's coordinates - if we do not have a peaking or shelving type,
  // we need to re-assign it to some value related to resonance or Q:
  if( filterToEdit->getMode() == MultiModeFilterParameters::MOOGISH_LOWPASS )
    y = (float) resoToY( filterToEdit->getResonance(), targetImage );
  else if( filterToEdit->currentModeSupportsGain() ) 
  {
    // keep y to be the transformed gain
  }
  else if( filterToEdit->currentModeSupportsQ() ) 
    y = (float) qToY( filterToEdit->getQ(), targetImage );
  else  
    y = (1.0/3.0) * getPlotHeight(targetImage);

  // draw the handle and a crosshair:
  g.fillEllipse((float) (x-dotRadius), (float) (y-dotRadius), 
    (float) (2*dotRadius), (float) (2*dotRadius) );
  g.setColour(graphColour.withAlpha(0.4f));
  float w = (float) getPlotWidth(targetImage);
  float h = (float) getPlotHeight(targetImage);
  g.drawLine(       0,(float)y,        w, (float)y, 1.f);  // horizontal
  g.drawLine((float)x,       0, (float)x,        h, 1.f);  // vertical
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// internal functions:

double MultiModeFreqResponseEditor::resoToY(double reso, juce::Image *targetImage)
{
  return (1.0-0.01*reso) * getPlotHeight(targetImage);
  //return dotRadius + (1.0-0.01*reso) * (getHeight()-2.f*dotRadius);
}

double MultiModeFreqResponseEditor::yToReso(double y, juce::Image *targetImage)
{
  return 100.0 * ( 1.0 - y/getPlotHeight(targetImage) );
  //return 100.0 * ( 1.0 + (dotRadius-y) / (getHeight()-2.0*dotRadius) );
}

double MultiModeFreqResponseEditor::qToY(double q, juce::Image *targetImage)
{
  return -expToLin(q, 0.5, 50.0, -getPlotHeight(targetImage), 0.0);
  //return 100.0;
}

double MultiModeFreqResponseEditor::yToQ(double y, juce::Image *targetImage)
{
  return linToExp(-y, -getPlotHeight(targetImage), 0.0, 0.5, 50.0);
}

//=================================================================================================


MultiModeFilterModuleEditor::MultiModeFilterModuleEditor(CriticalSection *newPlugInLock, 
  MultiModeFilterAudioModule* newMultiModeFilterAudioModule) 
  : AudioModuleEditor(newMultiModeFilterAudioModule)
{
  jassert(newMultiModeFilterAudioModule != NULL ); // you must pass a valid module here
  filterToEdit = newMultiModeFilterAudioModule->wrappedMultiModeFilter;

  setHeadlineText(juce::String("Filter"));

  stateWidgetSet->stateLabel->setVisible(false);
  webLink->setVisible(false);
  infoField->setVisible(false);

  /*
  addWidget( modeLabel = new RLabel(juce::String(T("ModeLabel")), juce::String(T("Type:"))) );
  modeLabel->setColour(Label::backgroundColourId, Colours::transparentWhite);
  modeLabel->setColour(Label::outlineColourId, Colours::transparentWhite);
  modeLabel->setDescription(T("Type/Mode of the filter."));
  */

  addWidget( modeComboBox = new RNamedComboBox(juce::String("ModeComboBox"), juce::String("Type:")) );
  modeComboBox->registerComboBoxObserver(this);
  modeComboBox->setDescription("Type/Mode of the filter.");
  modeComboBox->addItem( 0, "Bypass"               );  // \todo: use the values from the enum MultiModeFilterModes instead of magic numbers
  modeComboBox->addItem( 1, "Moogish Lowpass"      );
  modeComboBox->addItem( 2, "Lowpass 6 dB/oct"     );
  modeComboBox->addItem( 3, "Lowpass 12 dB/oct"    );
  modeComboBox->addItem( 4, "Highpass 6 dB/oct"    );
  modeComboBox->addItem( 5, "Highpass 12 dB/oct"   );
  modeComboBox->addItem( 6, "Bandpass 2*6 dB/oct"  );
  modeComboBox->addItem( 7, "Bandstop 2*6 dB/oct"  );
  modeComboBox->addItem( 8, "Peak/Dip"             );
  modeComboBox->addItem( 9, "Low Shelv 1st order"  );
  modeComboBox->addItem(10, "Low Shelv 2nd order"  );
  modeComboBox->addItem(11, "High Shelv 1st order" );
  modeComboBox->addItem(12, "High Shelv 2nd order" );
  modeComboBox->addItem(13, "Allpass 1st order"    );
  modeComboBox->addItem(14, "Allpass 2nd order"    );
  modeComboBox->addItem(15, "Morph Low/Peak/High"  );
  //modeComboBox->addItem(juce::String(T("Morph Low/Band/High")),      17);
  modeComboBox->selectItemByIndex(1, false);

  addWidget( twoStagesButton = new RButton(juce::String("2 Stages")) );
  twoStagesButton->addRButtonListener(this);
  twoStagesButton->setDescription(juce::String("Switch second filter stage on/off"));
  twoStagesButton->setClickingTogglesState(true);

  addWidget( freqSlider = new RSlider("FrequencySlider") );
  freqSlider->assignParameter(moduleToEdit->getParameterByName("Frequency"));
  freqSlider->setDescription("Characteristic frequency of the filter");
  freqSlider->setStringConversionFunction(&hertzToStringWithUnitTotal5);

  addWidget( freqByKeySlider = new RSlider("FrequencyByKeySlider") );
  freqByKeySlider->assignParameter(moduleToEdit->getParameterByName("FrequencyByKey"));
  freqByKeySlider->setSliderName("Key");
  freqByKeySlider->setDescription("Key tracking of the filter's frequency");
  freqByKeySlider->setStringConversionFunction(&percentToStringWithUnit1);

  addWidget( freqByVelSlider = new RSlider("FrequencyByVelSlider") );
  freqByVelSlider->assignParameter(moduleToEdit->getParameterByName("FrequencyByVel"));
  freqByVelSlider->setSliderName("Vel");
  freqByVelSlider->setDescription("Velocity tracking of the filter's frequency");
  freqByVelSlider->setStringConversionFunction(&percentToStringWithUnit1);

  addWidget( resoSlider = new RSlider("ResonanceSlider") );
  resoSlider->assignParameter(moduleToEdit->getParameterByName("Resonance"));
  resoSlider->setDescription("Resonance amount of the filter");
  resoSlider->setStringConversionFunction(&percentToStringWithUnit1);

  addWidget( qSlider = new RSlider("QSlider") );
  qSlider->assignParameter(moduleToEdit->getParameterByName("Q"));
  qSlider->setSliderName("Q");
  qSlider->setDescription("Quality factor of the filter");
  qSlider->setStringConversionFunction(&valueToString3);

  addWidget( driveSlider = new RSlider("DriveSlider") );
  driveSlider->assignParameter(moduleToEdit->getParameterByName("Drive"));
  driveSlider->setDescription("Drives the filter into distortion");
  driveSlider->setStringConversionFunction(&decibelsToStringWithUnit2);

  addWidget( orderSlider = new RSlider("OrderSlider") );
  orderSlider->assignParameter(moduleToEdit->getParameterByName("Order"));
  orderSlider->setDescription(
    "Selects the order of the filter - this affects the slope");
  orderSlider->setStringConversionFunction(&valueToString0);
  orderSlider->addListener(this);

  addWidget( gainSlider = new RSlider("GainSlider") );
  gainSlider->assignParameter(moduleToEdit->getParameterByName("Gain"));
  gainSlider->setSliderName("Gain");
  gainSlider->setDescription("Gain for peak and shelving filter types");
  gainSlider->setStringConversionFunction(&decibelsToStringWithUnit2);

  addWidget( morphSlider = new RSlider("MorphSlider") );
  morphSlider->assignParameter(moduleToEdit->getParameterByName("Morph") );
  morphSlider->setDescription("Morph between filter types");
  morphSlider->setStringConversionFunction(&valueToString2);
  //morphSlider->setRange(-0.99, 0.99, 0.01, -0.99);
  //morphSlider->setScaling(Parameter::LINEAR_BIPOLAR);
  //automatableSliders.addIfNotAlreadyThere(morphSlider);
  //morphSlider->addListener(this);

  //addWidget( transitionSlider = new RSlider("TransitionSlider") );
  //transitionSlider->assignParameter(moduleToEdit->getParameterByName("Transition") );
  //transitionSlider->setSliderName("Transition");
  //transitionSlider->setDescription("Determines the transition when morphing between filter types");
  //transitionSlider->setStringConversionFunction(&valueToString3);

  addWidget( preAllpassSlider = new RSlider("PreAllpassSlider") );
  preAllpassSlider->assignParameter(moduleToEdit->getParameterByName("PreAllpass") );
  preAllpassSlider->setSliderName("Allpass");
  preAllpassSlider->setDescription(
    "Applies a first order allpass before the actual filter to pre-shape the waveform");
  preAllpassSlider->setStringConversionFunction(&hertzToStringWithUnitTotal5);

  addWidget( makeUpSlider = new RSlider("MakeUpSlider") );
  makeUpSlider->assignParameter(moduleToEdit->getParameterByName("MakeUp") );
  makeUpSlider->setDescription(
    "Compensates the low frequency losses at high resonance via a gain factor");
  makeUpSlider->setStringConversionFunction(&percentToStringWithUnit0);
  makeUpSlider->addListener(this);

  //automatableSliders.addIfNotAlreadyThere(makeUpSlider);
  /*
  addWidget( freq2ScaleSlider = new RSlider(T("Freq2ScaleSlider")) );
  freq2ScaleSlider->addListener(this);
  freq2ScaleSlider->setSliderName(T("Freq2 Scale"));
  freq2ScaleSlider->setDescription(T("Scale factor of the second frequency with respect to the first"));
  freq2ScaleSlider->setStringConversionFunction(&valueToString3);
  freq2ScaleSlider->setRange(0.125, 8.0, 0.001, 2.0);
  freq2ScaleSlider->setScaling(Parameter::EXPONENTIAL);

  addWidget( freq2OffsetSlider = new RSlider(T("Freq2OffsetSlider")) );
  freq2OffsetSlider->addListener(this);
  freq2OffsetSlider->setSliderName(T("Freq2 Offset"));
  freq2OffsetSlider->setDescription(T("Offset (in Hz) of the second frequency with respect to the first"));
  freq2OffsetSlider->setStringConversionFunction(&hertzToStringWithUnitTotal5);
  freq2OffsetSlider->setRange(-2000.0, 2000.0, 0.01, 0.0);
  freq2OffsetSlider->setScaling(Parameter::LINEAR_BIPOLAR);

  addWidget( q2ScaleSlider = new RSlider(T("Q2ScaleSlider")) );
  q2ScaleSlider->addListener(this);
  q2ScaleSlider->setSliderName(T("Q2 Scale"));
  q2ScaleSlider->setDescription(T("Scale factor of the second q-factor with respect to the first"));
  q2ScaleSlider->setStringConversionFunction(&valueToString3);
  q2ScaleSlider->setRange(0.125, 8.0, 0.001, 2.0);
  q2ScaleSlider->setScaling(Parameter::EXPONENTIAL);

  addWidget( gain2ScaleSlider = new RSlider(T("Freq2ScaleSlider")) );
  gain2ScaleSlider->addListener(this);
  gain2ScaleSlider->setSliderName(T("Gain2 Scale"));
  gain2ScaleSlider->setDescription(T("Scale factor of the second gain with respect to the first"));
  gain2ScaleSlider->setStringConversionFunction(&valueToString3);
  gain2ScaleSlider->setRange(0.125, 8.0, 0.001, 2.0);
  gain2ScaleSlider->setScaling(Parameter::EXPONENTIAL);
  */

  frequencyResponseDisplay = new MultiModeFreqResponseEditor(juce::String("SpectrumEditor"));
  frequencyResponseDisplay->setFilterToEdit(filterToEdit);
  frequencyResponseDisplay->addChangeListener(this);
  frequencyResponseDisplay->assignParameterFreq( moduleToEdit->getParameterByName("Frequency"));
  frequencyResponseDisplay->assignParameterReso( moduleToEdit->getParameterByName("Resonance"));
  frequencyResponseDisplay->assignParameterQ(    moduleToEdit->getParameterByName("Q"));
  frequencyResponseDisplay->assignParameterGain( moduleToEdit->getParameterByName("Gain"));
  frequencyResponseDisplay->assignParameterMorph(moduleToEdit->getParameterByName("Morph") );
  addPlot( frequencyResponseDisplay );

  // customize the descriptions for the load/save buttons:
  stateWidgetSet->stateLoadButton->setDescription(   juce::String("Load filter settings from file"));
  stateWidgetSet->stateSaveButton->setDescription(   juce::String("Save filter settings to file"));
  stateWidgetSet->statePlusButton->setDescription(   juce::String("Skip to next filter settings file in current directory"));
  stateWidgetSet->stateMinusButton->setDescription(  juce::String("Skip to previous filter settings file in current directory"));
  stateWidgetSet->stateFileNameLabel->setDescription(juce::String("Name of current preset for the filter section (if any)"));

  isTopLevelEditor = false;

  updateWidgetsAccordingToState();
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// callbacks:

void MultiModeFilterModuleEditor::rButtonClicked(RButton *buttonThatWasClicked)
{
  if( filterToEdit == NULL )
    return;
  if( buttonThatWasClicked == twoStagesButton )
  {
    filterToEdit->useTwoStages(twoStagesButton->getToggleState());
    frequencyResponseDisplay->updatePlot();
  }
  moduleToEdit->markStateAsDirty();
  //sendChangeMessage();
}

void MultiModeFilterModuleEditor::rComboBoxChanged(RComboBox *rComboBoxThatHasChanged)
{
  if( filterToEdit == NULL )
    return;

  if( rComboBoxThatHasChanged == modeComboBox )
  {
    //filterToEdit->setMode(modeComboBox->getSelectedId());
    filterToEdit->setMode( stringToFilterModeIndex(modeComboBox->getText()) );
    updateWidgetArrangement();
    updateWidgetsAccordingToState();  
  }

  moduleToEdit->markStateAsDirty();
  //setPresetDirty();
  //frequencyResponseDisplay->updatePlot(); // already called from  updateWidgetsAccordingToState()
  //sendChangeMessage();
}

void MultiModeFilterModuleEditor::rSliderValueChanged(RSlider *sliderThatHasChanged)
{
  // there are some sliders for parameters which are not observed by the plot itself - when one of 
  // these changes, we do the update here:
  if(  sliderThatHasChanged == orderSlider || sliderThatHasChanged == makeUpSlider )
    frequencyResponseDisplay->updatePlot(); 
}


void MultiModeFilterModuleEditor::resized()
{
  linkPosition = INVISIBLE;
  AudioModuleEditor::resized();
  int x = 0;
  int y = 0;
  int w = getWidth();
  int h = getHeight();

  y = getPresetSectionBottom();
  frequencyResponseDisplay->setBounds(x, y+4, w, h-92);
  y = frequencyResponseDisplay->getBottom();

  modeComboBox->setBounds(x+4,  y+4, w/2-4, 20);

  y = modeComboBox->getBottom();
  freqSlider->setBounds(modeComboBox->getX(), y+4, w/2-4 , 16);

  y = freqSlider->getBottom()-2;
  w = freqSlider->getWidth();
  freqByKeySlider->setBounds(freqSlider->getX(),           y, w/2-2, 16);
  freqByVelSlider->setBounds(freqSlider->getX() + w/2 + 2, y, w/2-2, 16);

  y = modeComboBox->getY();
  w = getWidth();
  x = w/2;
  resoSlider->setBounds(x+4, y, w/2-8, 16);
  qSlider->setBounds(resoSlider->getBounds());

  y = resoSlider->getBottom();

  preAllpassSlider->setBounds(x+4,     y+4, w/4-4, 16);
  orderSlider->setBounds(     x+w/4+4, y+4, w/4-8, 16);
  twoStagesButton->setBounds(orderSlider->getBounds());

  gainSlider->setBounds(x+4, y+4, w/2-8, 16);
  morphSlider->setBounds(x+4, y+4, w/2-8, 16);

  y += 20;

  driveSlider->setBounds( x+4,     y+4, w/4-4, 16);
  makeUpSlider->setBounds(x+w/4+4, y+4, w/4-8, 16);
}

void MultiModeFilterModuleEditor::updateWidgetsAccordingToState()
{
  if( filterToEdit == NULL )
    return;

  //modeComboBox->setText(filterModeIndexToString(filterToEdit->getMode()), true); //old

  modeComboBox->selectItemFromText(filterModeIndexToString(filterToEdit->getMode()), false);

  // something to do here...?

  freqSlider->setValue(filterToEdit->getFrequencyNominal(),               false, false);
  freqByKeySlider->setValue(filterToEdit->getFrequencyByKey(),            false, false);
  freqByVelSlider->setValue(filterToEdit->getFrequencyByVel(),            false, false);
  resoSlider->setValue(filterToEdit->getResonance(),                      false, false);
  qSlider->setValue(filterToEdit->getQ(),                                 false, false);
  gainSlider->setValue(filterToEdit->getGain(),                           false, false);
  driveSlider->setValue(filterToEdit->getDrive(),                         false, false);
  morphSlider->setValue(filterToEdit->getMorph(),                         false, false);
  /*
  freq2ScaleSlider->setValue(
  filterToEdit->twoStageBiquad.getSecondFrequencyScaleFactor(),         false, false);
  freq2OffsetSlider->setValue(
  filterToEdit->twoStageBiquad.getSecondFrequencyOffset(),              false, false);
  q2ScaleSlider->setValue(
  filterToEdit->twoStageBiquad.getSecondQScaleFactor(),                 false, false);
  gain2ScaleSlider->setValue(
  filterToEdit->twoStageBiquad.getSecondGainScaleFactor(),              false, false);
  */
  orderSlider->setValue(filterToEdit->getOrder(),                         false, false);
  twoStagesButton->setToggleState(filterToEdit->usesTwoStages(),          false);
  preAllpassSlider->setValue(filterToEdit->getAllpassFreq(),              false, false);
  makeUpSlider->setValue(filterToEdit->getMakeUp(),                       false, false);

  updateWidgetArrangement();
  frequencyResponseDisplay->updatePlot();

  stateWidgetSet->updateStateNameField();
}

void MultiModeFilterModuleEditor::updateWidgetArrangement()
{
  if( filterToEdit == NULL )
    return;

  //int x, y, w, h;

  setWidgetsVisible(false);
  modeComboBox->setVisible(true);

  int mode = filterToEdit->getMode();


  // in bypass-mode, we dont need any widgets so we return:
  if( mode == rosic::MultiModeFilterParameters::BYPASS )
    return;

  // the frequency related sliders must be visible for all modes:
  freqSlider->setVisible(true);
  freqByKeySlider->setVisible(true);
  freqByVelSlider->setVisible(true);

  if( mode == rosic::MultiModeFilterParameters::MOOGISH_LOWPASS )
  {
    arrangeWidgetsForMoogishLowpassMode();
  }
  else  // twoStageBiquad
  {
    // all types of the TwoStageBiquad support the two-stages switch, so make the button visible:
    twoStagesButton->setVisible(true);

    if(    mode == rosic::MultiModeFilterParameters::LOWPASS_6 
      || mode == rosic::MultiModeFilterParameters::HIGHPASS_6 
      || mode == rosic::MultiModeFilterParameters::ALLPASS_1ST )
    {
      arrangeWidgetsForFirstOrderWithoutGain();
    }
    else if(    mode == rosic::MultiModeFilterParameters::LOWPASS_RBJ
      || mode == rosic::MultiModeFilterParameters::HIGHPASS_RBJ
      || mode == rosic::MultiModeFilterParameters::BANDPASS_RBJ
      || mode == rosic::MultiModeFilterParameters::BANDREJECT_RBJ
      || mode == rosic::MultiModeFilterParameters::ALLPASS_RBJ )
    {
      arrangeWidgetsForSecondOrderWithoutGain();
    }
    else if(    mode == rosic::MultiModeFilterParameters::LOW_SHELV_1ST
      || mode == rosic::MultiModeFilterParameters::HIGH_SHELV_1ST )
    {
      arrangeWidgetsForFirstOrderWithGain();
    }
    else if(    mode == rosic::MultiModeFilterParameters::LOW_SHELV_RBJ
      || mode == rosic::MultiModeFilterParameters::HIGH_SHELV_RBJ
      || mode == rosic::MultiModeFilterParameters::PEAK_OR_DIP_RBJ )
    {
      arrangeWidgetsForSecondOrderWithGain();
    }
    else if(    mode == rosic::MultiModeFilterParameters::MORPH_LP_PK_HP )
    {
      arrangeWidgetsForMorphableMode();
    }

    /*
    // the first order types don't support 'Q', for the others we must make the slider visible:
    if(    filterToEdit->getMode() != rosic::MultiModeFilterParameters::LOWPASS_6 
    && filterToEdit->getMode() != rosic::MultiModeFilterParameters::HIGHPASS_6 
    && filterToEdit->getMode() != rosic::MultiModeFilterParameters::LOW_SHELV_1ST 
    && filterToEdit->getMode() != rosic::MultiModeFilterParameters::HIGH_SHELV_1ST
    && filterToEdit->getMode() != rosic::MultiModeFilterParameters::ALLPASS_1ST )
    {
    qSlider->setVisible(true);
    }
    // the peak and shelving types support the gain parameter:
    if(    filterToEdit->getMode() == rosic::MultiModeFilterParameters::PEAK_OR_DIP_RBJ 
    || filterToEdit->getMode() == rosic::MultiModeFilterParameters::LOW_SHELV_1ST
    || filterToEdit->getMode() == rosic::MultiModeFilterParameters::LOW_SHELV_RBJ
    || filterToEdit->getMode() == rosic::MultiModeFilterParameters::HIGH_SHELV_1ST
    || filterToEdit->getMode() == rosic::MultiModeFilterParameters::HIGH_SHELV_RBJ   )
    {
    gainSlider->setVisible(true);
    }

    if(    filterToEdit->getMode() == rosic::MultiModeFilterParameters::MORPH_LP_BP_HP
    || filterToEdit->getMode() == rosic::MultiModeFilterParameters::MORPH_LP_PK_HP )
    {
    morphSlider->setVisible(true);
    //twoStagesButton->setVisible(true);
    //transitionSlider->setVisible(true);
    y = morphSlider->getBottom();
    x = morphSlider->getRight()-64;
    w = 64;
    h = 16;
    twoStagesButton->setBounds(x, y+4, w, h);
    int dummy = 0;
    }
    */

  }

}



/*
void MultiModeFilterEditor::arrangeCommonWidgets()
{

}
*/

void MultiModeFilterModuleEditor::arrangeWidgetsForMoogishLowpassMode()
{
  resoSlider->setVisible(true);
  driveSlider->setVisible(true);
  orderSlider->setVisible(true);
  preAllpassSlider->setVisible(true);
  makeUpSlider->setVisible(true);
}

void MultiModeFilterModuleEditor::arrangeWidgetsForFirstOrderWithoutGain()
{
  int x = modeComboBox->getRight()+4;
  int y = modeComboBox->getY();
  twoStagesButton->setVisible(true);
  twoStagesButton->setBounds(x+4, y, 80, 16);
}

void MultiModeFilterModuleEditor::arrangeWidgetsForSecondOrderWithoutGain()
{
  int x = modeComboBox->getRight()+4;
  int y = modeComboBox->getY();
  int w = freqSlider->getWidth();

  twoStagesButton->setVisible(true);
  twoStagesButton->setBounds(x+4, y, 80, 16);
  y += 20;
  w  = getWidth()-x;
  qSlider->setVisible(true);
  qSlider->setBounds(x+4, y, w-8, 16);
}

void MultiModeFilterModuleEditor::arrangeWidgetsForFirstOrderWithGain()
{
  arrangeWidgetsForFirstOrderWithoutGain();
  gainSlider->setVisible(true);
  gainSlider->setBounds(qSlider->getX(), qSlider->getY(), qSlider->getWidth(), 16);
}

void MultiModeFilterModuleEditor::arrangeWidgetsForSecondOrderWithGain()
{
  arrangeWidgetsForSecondOrderWithoutGain();
  gainSlider->setVisible(true);
  gainSlider->setBounds(qSlider->getX(), qSlider->getY()+20, qSlider->getWidth(), 16);
}

void MultiModeFilterModuleEditor::arrangeWidgetsForMorphableMode()
{
  arrangeWidgetsForSecondOrderWithoutGain();
  morphSlider->setVisible(true);
  morphSlider->setBounds(qSlider->getX(), qSlider->getY()+20, qSlider->getWidth(), 16);
}
