
// construction/destruction:

AciDevilAudioModule::AciDevilAudioModule(CriticalSection *newPlugInLock, 
  rosic::AciDevil *aciDevilToWrap) : AudioModuleWithMidiIn(newPlugInLock) 
{
  jassert(aciDevilToWrap != NULL); // you must pass a valid rosic-object to the constructor
  wrappedAciDevil = aciDevilToWrap;

  // factor out common code of both construtors into init function:
  setModuleName(juce::String("AciDevil"));
  setActiveDirectory(getApplicationDirectory() + juce::String("/AciDevilPresets") );
  initializeAutomatableParameters();
  sequencerModule = new AcidSequencerAudioModule(lock, &wrappedAciDevil->sequencer);
  sequencerModule->setModuleName(juce::String("Sequencer"));
  addChildAudioModule(sequencerModule);
}

AciDevilAudioModule::AciDevilAudioModule(CriticalSection *newPlugInLock) 
  : AudioModuleWithMidiIn(newPlugInLock) 
{
  wrappedAciDevil = new rosic::AciDevil;
  wrappedAciDevilIsOwned = true;

  // factor out common code of both construtors into init function:
  setModuleName(juce::String("AciDevil"));
  setActiveDirectory(getApplicationDirectory() + juce::String("/AciDevilPresets") );
  initializeAutomatableParameters();
  sequencerModule = new AcidSequencerAudioModule(lock, &wrappedAciDevil->sequencer);
  sequencerModule->setModuleName(juce::String("Sequencer"));
  addChildAudioModule(sequencerModule);
}

AciDevilAudioModule::~AciDevilAudioModule()
{
  if(wrappedAciDevilIsOwned)
    delete wrappedAciDevil;
}

AudioModuleEditor* AciDevilAudioModule::createEditor()
{
  return new jura::AciDevilModuleEditor(lock, this); // get rid of passing the lock
}

// automation:

void AciDevilAudioModule::parameterChanged(Parameter* parameterThatHasChanged)
{

  if( wrappedAciDevil == NULL )
    return;

  int    index = getIndexOfParameter(parameterThatHasChanged);
  double value = parameterThatHasChanged->getValue();

  switch( index )
  {
  case   0: wrappedAciDevil->setMasterLevel(value);           break;
  case   1: wrappedAciDevil->setAccent(value);                break;
  case   2: wrappedAciDevil->setSlideTime(value);             break;
  case   3: wrappedAciDevil->setWaveform(value);              break;
  case   4: wrappedAciDevil->oscillator.setPulseWidth(value); break;
  case   5: wrappedAciDevil->setSubOscLevel(value);           break;
  case   6: wrappedAciDevil->setSubOscWaveform(value);        break;
  case   7: wrappedAciDevil->setCutoff(value);                break;
  case   8: wrappedAciDevil->filter.setResonance(value);      break;
  case   9: wrappedAciDevil->filter.setMode((int) value);     break;
  case  10: wrappedAciDevil->setEnvMod(value);                break;
  case  11: wrappedAciDevil->setNormalDecay(value);           break;
  case  12: wrappedAciDevil->setAccentDecay(value);           break;
  case  13: wrappedAciDevil->setNormalAttack(value);          break;
  case  14: wrappedAciDevil->setAccentAttack(value);          break;
    // 15 - upwardFraction
  case  16: wrappedAciDevil->setAmpDecay(value);              break;
  case  17: wrappedAciDevil->setAmpSustain(value);            break;
  case  18: wrappedAciDevil->setAmpRelease(value);            break;
  case  19: wrappedAciDevil->setClipperDrive(value);          break;
    //....

  default:
    {
      // do nothing
    }
  } // end of switch( parameterIndex )

}

//-------------------------------------------------------------------------------------------------
// internal functions:

void AciDevilAudioModule::initializeAutomatableParameters()
{
  AutomatableParameter* p;

  // #000:
  p = new AutomatableParameter(lock, "MasterLevel", -60.0, 0.0, 0.1, -12.0, Parameter::LINEAR);
  addObservedParameter(p);

  // #001:
  p = new AutomatableParameter(lock, "Accent", 0.0, 100.0, 0.1, 50.0, Parameter::LINEAR);
  addObservedParameter(p);

  // #002:
  p = new AutomatableParameter(lock, "SlideTime", 1.0, 500.0, 0.1, 60.0, Parameter::EXPONENTIAL);
  addObservedParameter(p);

  // #003:
  p = new AutomatableParameter(lock, "Waveform", 0.0, 1.0, 0.01, 0.0, Parameter::LINEAR);
  addObservedParameter(p);

  // #004:
  p = new AutomatableParameter(lock, "PulseWidth", 1.0, 100.0, 0.1, 45.0, Parameter::LINEAR);
  addObservedParameter(p);


  // #005:
  p = new AutomatableParameter(lock, "SubOscLevel", -60.0, 0.0, 0.1, -60.0, Parameter::LINEAR);
  addObservedParameter(p);

  // #006:
  p = new AutomatableParameter(lock, "SubOscWaveform", 0.0, 1.0, 0.01, 1.0, Parameter::LINEAR);
  addObservedParameter(p);

  // #007:
  p = new AutomatableParameter(lock, "Cutoff", 200.0, 10000.0, 0.0, 300.0, Parameter::EXPONENTIAL, 74);
  addObservedParameter(p);

  // #008:
  p = new AutomatableParameter(lock, "Resonance", 0.0, 100.0, 0.1, 50.0, Parameter::LINEAR, 71);
  addObservedParameter(p);

  // #009:
  p = new AutomatableParameter(lock, juce::String("FilterMode"), 0.0, 14.0, 1.0, 1.0, Parameter::STRING);
  p->addStringValue("Flat");
  p->addStringValue("Lowpass 6");
  p->addStringValue("Lowpass 12");
  p->addStringValue("Lowpass 18");
  p->addStringValue("Lowpass 24");
  p->addStringValue("Highpass 6");
  p->addStringValue("Highpass 12");
  p->addStringValue("Highpass 18");
  p->addStringValue("Highpass 24");
  p->addStringValue("Bandpass 12+12");
  p->addStringValue("Bandpass 6+18");
  p->addStringValue("Bandpass 18+6");
  p->addStringValue("Bandpass 6+12");
  p->addStringValue("Bandpass 12+6");
  p->addStringValue("Bandpass 6+6");
  p->setValue(3.0, false, false);
  addObservedParameter(p);

  // #010:
  p = new AutomatableParameter(lock, "EnvMod", 0.0, 80.0, 0.1, 12.0, Parameter::LINEAR, 81);
  addObservedParameter(p);

  // #011:
  p = new AutomatableParameter(lock, "NormalDecay", 30.0, 3000.0, 0.1, 200.0, Parameter::EXPONENTIAL);
  addObservedParameter(p);

  // #012:
  p = new AutomatableParameter(lock, "AccentDecay", 30.0, 300.0, 0.1, 60.0, Parameter::EXPONENTIAL);
  addObservedParameter(p);

  // #013:
  p = new AutomatableParameter(lock, "NormalAttack", 3.0, 50.0, 0.1, 3.0, Parameter::EXPONENTIAL);
  addObservedParameter(p);

  // #014:
  p = new AutomatableParameter(lock, "AccentAttack", 3.0, 50.0, 0.1, 10.0, Parameter::EXPONENTIAL);
  addObservedParameter(p);

  // #015:
  p = new AutomatableParameter(lock, "UpwardFraction", 0.0, 100.0, 0.1, 66.6, Parameter::LINEAR);
  addObservedParameter(p);

  // #016:
  p = new AutomatableParameter(lock, "AmpDecay", 3.0, 3000.0, 0.1, 1230.0, Parameter::EXPONENTIAL);
  addObservedParameter(p);

  // #017:
  p = new AutomatableParameter(lock, "AmpSustain", -60.0, 0.0, 0.1, -60.0, Parameter::LINEAR);
  addObservedParameter(p);

  // #018:
  p = new AutomatableParameter(lock, "AmpRelease", 0.3, 50.0, 0.1, 0.5, Parameter::EXPONENTIAL);
  addObservedParameter(p);

  // #019:
  p = new AutomatableParameter(lock, "DistortionDrive", -24.0, 60.0, 0.1, 0.0, Parameter::LINEAR);
  addObservedParameter(p);

  // make a call to parameterChanged for each parameter in order to set up the DSP-core to reflect 
  // the values the automatable parameters:
  for(int i=0; i < (int) parameters.size(); i++ )
    parameterChanged(parameters[i]);
}

//=================================================================================================



AciDevilModuleEditor::AciDevilModuleEditor(CriticalSection *newPlugInLock, AciDevilAudioModule* newAciDevilAudioModule) 
  : AudioModuleEditor(newAciDevilAudioModule)
{
  setHeadlineStyle(MAIN_HEADLINE);

  // assign the pointer to the rosic::AciDevil object to be used as aduio engine:
  jassert(newAciDevilAudioModule != NULL ); // you must pass a valid module here
  aciDevilModuleToEdit = newAciDevilAudioModule;

  addWidget( globalLabel = new RTextField( juce::String("Global")) );
  //globalLabel->setJustificationType(Justification::centred);
  globalLabel->setDescription("Global parameters");
  globalLabel->setDescriptionField(infoField);

  addWidget( masterLevelSlider = new RSlider("MasterLevelSlider") );
  masterLevelSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("MasterLevel") );
  masterLevelSlider->setSliderName(juce::String("Level"));
  masterLevelSlider->setDescription(juce::String("Master level in decibels"));
  masterLevelSlider->setStringConversionFunction(decibelsToStringWithUnit1);
  masterLevelSlider->setDescriptionField(infoField);

  addWidget( accentSlider = new RSlider("AccentSlider") );
  accentSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("Accent") );
  accentSlider->setDescription(juce::String("Accent in percent"));
  accentSlider->setStringConversionFunction(percentToStringWithUnit1);
  accentSlider->setDescriptionField(infoField);

  addWidget( slideTimeSlider = new RSlider ("SlideTimeSlider") );
  slideTimeSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("SlideTime") );
  slideTimeSlider->setSliderName(juce::String("Slide"));
  slideTimeSlider->setDescription(juce::String("Slide time in milliseconds"));
  slideTimeSlider->setStringConversionFunction(millisecondsToStringWithUnit2);
  slideTimeSlider->setDescriptionField(infoField);

  addWidget( oscLabel = new RTextField( juce::String("Oscillator")) );
  oscLabel->setJustification(Justification::centred);
  oscLabel->setDescription("Oscillator parameters");
  oscLabel->setDescriptionField(infoField);

  addWidget( waveformSlider = new RSlider ("WaveformSlider") );
  waveformSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("Waveform") );
  waveformSlider->setSliderName(juce::String("Saw/Pulse"));
  waveformSlider->setDescription(juce::String("Mix between saw- and pulse-wave for main oscillator"));
  waveformSlider->setStringConversionFunction(ratioToString0);
  waveformSlider->setDescriptionField(infoField);

  addWidget( subOscLabel = new RTextField( juce::String("SubOsc:")) );
  subOscLabel->setJustification(Justification::centredLeft);
  subOscLabel->setDescription("Sub-oscillator settings");
  subOscLabel->setDescriptionField(infoField);

  addWidget( subOscLevelSlider = new RSlider ("SubOscLevelSlider") );
  subOscLevelSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("SubOscLevel") );
  subOscLevelSlider->setSliderName(juce::String("Level"));
  subOscLevelSlider->setDescription(juce::String("Sub-oscillator level in decibels"));
  subOscLevelSlider->setStringConversionFunction(decibelsToStringWithUnit1);
  subOscLevelSlider->setDescriptionField(infoField);

  addWidget( subOscWaveformSlider = new RSlider("SubOscWaveformSlider") );
  subOscWaveformSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("SubOscWaveform") );
  subOscWaveformSlider->setSliderName(juce::String("Saw/Pulse"));
  subOscWaveformSlider->setDescription(juce::String("Mix between saw- and pulse-wave for suboscillator"));
  subOscWaveformSlider->setStringConversionFunction(ratioToString0);
  subOscWaveformSlider->setDescriptionField(infoField);


  addWidget( filterLabel = new RTextField( juce::String("Filter")) );
  filterLabel->setJustification(Justification::centred);
  filterLabel->setDescription("Filter");
  filterLabel->setDescriptionField(infoField);

  addWidget( cutoffSlider = new RSlider("CutoffSlider") );
  cutoffSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("Cutoff") );
  cutoffSlider->setDescription(juce::String("Filter cutoff frequency in Hz"));
  cutoffSlider->setStringConversionFunction(hertzToStringWithUnitTotal5);
  cutoffSlider->setDescriptionField(infoField);

  addWidget( resonanceSlider = new RSlider ("ResonanceSlider") );
  resonanceSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("Resonance") );
  resonanceSlider->setDescription(juce::String("Resonance in percent"));
  resonanceSlider->setStringConversionFunction(percentToStringWithUnit1);
  resonanceSlider->setDescriptionField(infoField);

  addWidget( filterModeLabel = new RTextField( juce::String("Mode:")) );
  filterModeLabel->setJustification(Justification::centredLeft);
  filterModeLabel->setDescription("Choose the filter mode");
  filterModeLabel->setDescription("Mode:");
  filterModeLabel->setDescriptionField(infoField);

  addWidget( filterModeBox = new RComboBox(juce::String("FilterModeComboBox")) );
  filterModeBox->assignParameter( moduleToEdit->getParameterByName("FilterMode") );
  filterModeBox->setDescription(filterModeLabel->getDescription());
  filterModeBox->setDescriptionField(infoField);

  addWidget( envModSlider = new RSlider("EnvModSlider") );
  envModSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("EnvMod") );
  envModSlider->setDescription(juce::String("Amount of modulation of the cutoff frequency in semitones"));
  envModSlider->setStringConversionFunction(semitonesToStringWithUnit1);
  envModSlider->setDescriptionField(infoField);


  addWidget( filterEnvLabel = new RTextField( juce::String("Filter Envelope")) );
  filterEnvLabel->setJustification(Justification::centred);
  filterEnvLabel->setDescription("Filter envelope parameters");
  filterEnvLabel->setDescriptionField(infoField);

  addWidget( normalLabel = new RTextField( juce::String("Normal:")) );
  normalLabel->setJustification(Justification::centredLeft);
  normalLabel->setDescription("Time values for normal (un-accented) notes");
  normalLabel->setDescriptionField(infoField);

  addWidget( normalDecaySlider = new RSlider ("NormalDecaySlider") );
  normalDecaySlider->assignParameter( aciDevilModuleToEdit->getParameterByName("NormalDecay") );
  normalDecaySlider->setSliderName(juce::String("Decay"));
  normalDecaySlider->setDescription(juce::String("Decay time for normal (un-accented) notes in milliseconds."));
  normalDecaySlider->setStringConversionFunction(millisecondsToStringWithUnit2);
  normalDecaySlider->setDescriptionField(infoField);

  addWidget( normalAttackSlider = new RSlider("NormalAttackSlider") );
  normalAttackSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("NormalAttack") );
  normalAttackSlider->setSliderName(juce::String("Attack"));
  normalAttackSlider->setDescription(juce::String("Attack time for normal (un-accented) notes in milliseconds."));
  normalAttackSlider->setStringConversionFunction(millisecondsToStringWithUnit2);
  normalAttackSlider->setDescriptionField(infoField);


  addWidget( accentLabel = new RTextField( juce::String("Accent:")) );
  accentLabel->setJustification(Justification::centredLeft);
  accentLabel->setDescription("Time values for accented notes");
  accentLabel->setDescriptionField(infoField);

  addWidget( accentDecaySlider = new RSlider ("AccentDecaySlider") );
  accentDecaySlider->assignParameter( aciDevilModuleToEdit->getParameterByName("AccentDecay") );
  accentDecaySlider->setSliderName(juce::String("Decay"));
  accentDecaySlider->setDescription(juce::String("Decay time for accented notes in milliseconds."));
  accentDecaySlider->setStringConversionFunction(millisecondsToStringWithUnit2);
  accentDecaySlider->setDescriptionField(infoField);

  addWidget( accentAttackSlider = new RSlider ("AccentAttackSlider") );
  accentAttackSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("AccentAttack") );
  accentAttackSlider->setSliderName(juce::String("Attack"));
  accentAttackSlider->setDescription(juce::String("Attack time for accented notes in milliseconds."));
  accentAttackSlider->setStringConversionFunction(millisecondsToStringWithUnit2);
  accentAttackSlider->setDescriptionField(infoField);

  addWidget( ampLabel = new RTextField( juce::String("Amplifier")) );
  ampLabel->setJustification(Justification::centred);
  ampLabel->setDescription("Amplide envelope and distortion parameters");
  ampLabel->setDescriptionField(infoField);

  addWidget( ampDecaySlider = new RSlider("AmpDecaySlider") );
  ampDecaySlider->assignParameter( aciDevilModuleToEdit->getParameterByName("AmpDecay") );
  ampDecaySlider->setSliderName(juce::String("Decay"));
  ampDecaySlider->setDescription(juce::String("Decay time for the amplitude envelope in milliseconds."));
  ampDecaySlider->setStringConversionFunction(millisecondsToStringWithUnit2);
  ampDecaySlider->setDescriptionField(infoField);

  addWidget( ampSustainSlider = new RSlider("AmpSustainSlider") );
  ampSustainSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("AmpSustain") );
  ampSustainSlider->setSliderName(juce::String("Sustain"));
  ampSustainSlider->setDescription(juce::String("Sustain level for the amplitude envelope in decibels."));
  ampSustainSlider->setStringConversionFunction(decibelsToStringWithUnit1);
  ampSustainSlider->setDescriptionField(infoField);

  addWidget( ampReleaseSlider = new RSlider("AmpReleaseSlider") );
  ampReleaseSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("AmpRelease") );
  ampReleaseSlider->setSliderName(juce::String("Release"));
  ampReleaseSlider->setDescription(juce::String("Release time for the amplitude envelope in milliseconds."));
  ampReleaseSlider->setStringConversionFunction(millisecondsToStringWithUnit2);
  ampReleaseSlider->setDescriptionField(infoField);

  addWidget( distortionDriveSlider = new RSlider("DistortionDriveSlider") );
  distortionDriveSlider->assignParameter( aciDevilModuleToEdit->getParameterByName("DistortionDrive") );
  distortionDriveSlider->setSliderName(juce::String("Drive"));
  distortionDriveSlider->setDescription(juce::String("Drive for the distortion unit in decibels."));
  distortionDriveSlider->setStringConversionFunction(decibelsToStringWithUnit1);
  distortionDriveSlider->setDescriptionField(infoField);

  sequencerEditor = new AcidSequencerModuleEditor(lock, aciDevilModuleToEdit->sequencerModule);
  addChildEditor( sequencerEditor );
  sequencerEditor->setDescriptionField(infoField, true);

  // set up the widgets:
  updateWidgetsAccordingToState();

  setSize(800, 400);
}

//-------------------------------------------------------------------------------------------------
// setup:



//-------------------------------------------------------------------------------------------------
// callbacks:

void AciDevilModuleEditor::updateWidgetsAccordingToState()
{
  AudioModuleEditor::updateWidgetsAccordingToState();
  sequencerEditor->updateWidgetsAccordingToState();
}

void AciDevilModuleEditor::paint(Graphics &g)
{
  AudioModuleEditor::paint(g);
}

void AciDevilModuleEditor::resized()
{
  AudioModuleEditor::resized();
  int x = 0;
  int y = 0;
  int w = getWidth();
  int h = getHeight();

  y = getHeadlineBottom()+4;
  w = 220;
  h = 120;

  guiLayoutRectangles.clear();
  globalRectangle.setBounds(x, y, w, h);
  x = globalRectangle.getRight();
  w = 140;
  oscRectangle.setBounds(x, y, w, h);
  x = oscRectangle.getRight();
  w = 140;
  filterRectangle.setBounds(x, y, w, h);
  x = filterRectangle.getRight();
  w = 140;
  filterEnvRectangle.setBounds(x, y, w, h);
  x = filterEnvRectangle.getRight();
  w = 140;
  ampRectangle.setBounds(x, y, w, h);
  guiLayoutRectangles.add(globalRectangle);
  guiLayoutRectangles.add(oscRectangle);
  guiLayoutRectangles.add(filterRectangle);
  guiLayoutRectangles.add(filterEnvRectangle);
  guiLayoutRectangles.add(ampRectangle);


  x = globalRectangle.getX();
  y = globalRectangle.getY();
  w = globalRectangle.getWidth();

  stateWidgetSet->setLayout(StateLoadSaveWidgetSet::LABEL_AND_BUTTONS_ABOVE);
  stateWidgetSet->setBounds(x+4, y+4, w-8, 32);
  y = stateWidgetSet->getBottom()+4;

  y = stateWidgetSet->getBottom() + 4 + 32; // leave space for tuning widgets

  masterLevelSlider->setBounds(x+4, y+4, w-8, 16);
  y = masterLevelSlider->getBottom();
  accentSlider->setBounds(    x+4,     y+4, w/2-8, 16);
  slideTimeSlider->setBounds( x+w/2+4, y+4, w/2-8, 16);


  x = oscRectangle.getX();
  y = oscRectangle.getY();
  w = oscRectangle.getWidth();
  oscLabel->setBounds(x, y+2, w, 16);
  y = oscLabel->getBottom();
  waveformSlider->setBounds(x+4, y+4, w-8, 16);
  y += 24;
  subOscLabel->setBounds(x+4, y+4, w-8, 16);
  y += 16;

  subOscLevelSlider->setBounds(x+4, y+4, w-8, 16);
  y += 14;
  subOscWaveformSlider->setBounds(x+4, y+4, w-8, 16);

  x = filterRectangle.getX();
  y = filterRectangle.getY();
  w = filterRectangle.getWidth();
  filterLabel->setBounds(x, y+2, w, 16);
  y = filterLabel->getBottom();
  cutoffSlider->setBounds(x+4, y+4, w-8, 16);
  y += 20;
  resonanceSlider->setBounds(x+4, y+4, w-8, 16);
  y += 20;
  filterModeLabel->setBounds(x+4,    y+4, 40,     16);
  filterModeBox->setBounds(  x+40+4, y+4, w-40-8, 16);
  y += 20;
  envModSlider->setBounds(x+4, y+4, w-8, 16);

  x = filterEnvRectangle.getX();
  y = filterEnvRectangle.getY();
  w = filterEnvRectangle.getWidth();
  filterEnvLabel->setBounds(x, y+2, w, 16);
  y = filterEnvLabel->getBottom();
  normalLabel->setBounds(x+4, y+4, w-8, 16);
  y += 16;
  normalDecaySlider->setBounds(x+4, y+4, w-8, 16);
  y += 14;
  normalAttackSlider->setBounds(x+4, y+4, w-8, 16);
  y += 16;
  accentLabel->setBounds(x+4, y+4, w-8, 16);
  y += 16;
  accentDecaySlider->setBounds(x+4, y+4, w-8, 16);
  y += 14;
  accentAttackSlider->setBounds(x+4, y+4, w-8, 16);


  x = ampRectangle.getX();
  y = ampRectangle.getY();
  w = ampRectangle.getWidth();
  ampLabel->setBounds(x, y+2, w, 16);
  y = ampLabel->getBottom();
  ampDecaySlider->setBounds(x+4, y+4, w-8, 16);
  y += 20;
  ampSustainSlider->setBounds(x+4, y+4, w-8, 16);
  y += 20;
  ampReleaseSlider->setBounds(x+4, y+4, w-8, 16);
  y += 24;
  distortionDriveSlider->setBounds(x+4, y+4, w-8, 16);

  y = globalRectangle.getBottom();
  w = filterRectangle.getRight();
  sequencerEditor->setBounds(0, y, w, 252);
}

