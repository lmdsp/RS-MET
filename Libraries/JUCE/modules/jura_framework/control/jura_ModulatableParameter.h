#ifndef jura_ModulatableParameter_h
#define jura_ModulatableParameter_h

/*

The Modulation System


How to use it:

Client code that wants to use the modulation system for plugin parameters must do the following 
things:

-derive its AudioModule from ModulatableAudioModule (or from a subclass therof)
-somewhere have a ModulationManager object lying around, a pointer to that object should be passed
 to the constructor call of the ModulatableAudioModule baseclass
-use objects of type ModulatableParameter for its parameters
-(not yet:) use objects of type ModulatableSlider for the sliders of the to-be-modulated parameters
-every ModulationSource that should be available must be registered with the ModulationManager
 object
-before producing a sample, your AudioModule must call applyModulations on the ModulationManager
 object


How it works:

There are 4 main classes that are relevant for the system: ModulationSource, ModulationTarget, 
ModulationConnection and ModulationManager. ModulationSource and ModulationTarget have a common 
baseclass ModulationParticipant for factoring out some common stuff but the client is usually not 
concerned with this. Sources are things like envelope generators, LFOs, etc. For example, to make 
an envelope generator available for the modulation system, it would have to be a subclass of 
ModulationSource. The subclass is then required to implement the purely virtual 
updateModulationValue() method. There, it is supposed to compute a new output sample of the 
modulator and assign the "modValue" member variable to that value (that value will later be 
gathered by the ModulationManager and used for modulating things). ModulationTargets are typically
parameters of some audio-processing algorithm such as the cutoff frequency of a filter. Typically,
clients will only use ModulatableParameter which is a subclass of ModulationTarget and Parameter.
A ModulationTarget subclass is required to implement doModulationUpdate() which is also called per 
sample. There, they must use the "modulatedValue" member (which was set up by the ModulationManager 
and now - inside the doModulationUpdate call - it contains the parameter value with all modulations 
applied. A ModulatableParameter will call its valueChangeCallback (inherited from Parameter), 
which, for example, may cause a call to setCutoff(modulatedValue) on some core filter object. 
ModulationConnection represents a connection between a given ModualtionSource and ModulationTarget. 
It also contains the strength of the connection, i.e. the depth by which some source modulates 
some target. The ModulationManager maintains arrays of all available sources and targets and, 
importantly, all the connections. It can also create new and remove existing connections. It 
is also responsible for calling updateModulationValue on all registered ModulationSources and 
doModulationUpdate on all affected ModulationTargets - between these sequences of calls, it gathers
up all the source-outputs, and applies them to their targets - as defined via the connections.



old:

Idea to extend the same concept of MetaParameters to a modulation system for synthesizers:

ModulationSource:
-provide interface to pull out a current value
-subclasses can be fixed values (like gui parameter sliders), envelope generators, low frequency 
 oscillators, step sequencers, midi-note values, etc.
-At each sample, the first thing to do is to compute all current values of all existing modulation 
 sources. This should be done in a central place in some higher level object (maybe some subclass 
 of AudioModule)

ModulationTarget:
-is typically some (lower level) dsp algorithm parameter such as a cutoff frequency of a filter
-keeps a list of assigned sources
-keeps a pointer to ModulationManager where it can sign up to receive inputs from sources

ModulationManager:
-allows ModulationsTargets to de/register themselves to ModulationSources

similarities to MetaParameters:
-needs similar sign-up system on the gui (right-click - assign, etc.)

differences to MetaParameters:
-each modulation target can be connected to any number of modulation sources (many-to-many instead 
 of one-to-many)
-uses pull rather than push mechanism to update the dependent parameter

feedback:
-it is desirable to be able for (parameters of) ModulationsSources themselves be modulated by 
 other ModulationSources or even by their own output
-maybe the parameters of a ModulationSource (like an LFO freq) should be subclasses of 
 ModulationTarget ...or just BE a ModulationTarget object - or maybe we'll need a class
 ModulatableParameter

maybe make the class hierarchy like this:
Parameter <- MetaControlledParameter <- ModulatableParameter <- PolyphonicParameter

ToDo:
-It doesn't work in Elan's SpiralGenerator - why? maybe it has to do with creation order? There,
 the modulators whould modulate parameters of their parent module, in chainer it's the sibling
 modules - and no sources or targets are created in the constructor
 Solution:
 -call setModulationManager in constructor b4 creating sources and targets
 -..
-figure out what happens if the user changes the range of depthParam - how will this affect the 
 meta-value, how can we make sure that the depth parameter is always consistent with its attached 
 metaparameter? how its patch recall affected?
-optimize, where possible - maybe the scaler can be multiplied into the depth (maybe use a 
 scaledDepth member)
 -maybe the ModulationManager shhould maintain an array usedSources and itereate only over that in
  applyModulations (similar to the treatment of affectedTargets - maybe call the arrays 
  connectedTargets and connectedSources to make the parallelism obvious)
-let the depth be controlled by key and/or velocity - or better, make key and vel available as
 ModulationSource and let Depth-parameters be ModulationTargets (i.e. ModulatableParameters)
 ...or maybe not the midi key/vel values but something more physical, for example Note-Frequency
 and Amplitude, i.e. freq = pitchToFreq(key+pitchbend), amp = vel/127.

*/

// forward declarations:
class ModulationManager; 
class ModulationSource;
class ModulationTarget;
class ModulationConnection;

//=================================================================================================

/** Baseclass for participants in the modulation system. Subclasses are ModulationSource and 
ModulationTarget and this class factors out what they have in common, which is mainly the access 
functions to the ModulationManager. There can also be other subclasses that need a convenient 
access point to a ModulationManager, such as an AudioModule subclass that contains modulatable
parameters. */

class JUCE_API ModulationParticipant
{

public:

  /** Constructor */
  ModulationParticipant(ModulationManager* managerToUse = nullptr) 
  {
    modManager = managerToUse;
  }

  /** Destructor */
  virtual ~ModulationParticipant() {}

  /** Sets up the ModulationManager that should be used for registering ModulationSources and 
  ModulationTargets. Should be called sometime soon after construction. */
  void setModulationManager(ModulationManager* managerToUse)
  {
    modManager = managerToUse;
  }
  // maybe get rid of this and demand it to be passed to the constructor

  /** Returns a pointer to the ModulationManager object that is used for registering 
  ModulationSources and ModulationTargets. */
  ModulationManager* getModulationManager()
  {
    return modManager;
  }

  /** Registers the given ModulationSource to make it available to ModulationTargets. */
  void registerModulationSource(ModulationSource* source);

  /** De-registers a ModulationSource. */
  void deRegisterModulationSource(ModulationSource* source);

  /** Registers the given ModulationTarget. */
  void registerModulationTarget(ModulationTarget* target);

  /** De-registers a ModulationTarget. */
  void deRegisterModulationTarget(ModulationTarget* target);

  /** Returns a reference to the array of available ModulationSource objects. */
  const std::vector<ModulationSource*>& getAvailableModulationSources();

  /** Returns a reference to the array of available ModulationTarget objects. */
  const std::vector<ModulationTarget*>& getAvailableModulationTargets();

  /** Returns a reference to the array of ModulationConnectionTarget objects. */
  const std::vector<ModulationConnection*>& getModulationConnections();


protected:

  ModulationManager* modManager = nullptr;


  static std::vector<ModulationSource*> dummySources;
  static std::vector<ModulationTarget*> dummyTargets;
  static std::vector<ModulationConnection*> dummyConnections;
  // These are empty dummy arrays to which references will be returned by 
  // getAvailableModulationSources/Targets in case our modManager is a nullptr.  This is somehow
  // ugly design, maybe use the "Null Object" pattern instead:
  // https://sourcemaking.com/design_patterns/null_object)
  // somewhere, we should have a default "null" ModulationManager object lying around to which
  // our pointer is initialized



  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationParticipant)
};

//=================================================================================================

/** Baseclass for modulation sources. */

class JUCE_API ModulationSource : public ModulationParticipant
{

public:

  /** Constructor */
  ModulationSource(ModulationManager* managerToUse = nullptr) 
    : ModulationParticipant(managerToUse) {}

  /** Destructor */
  virtual ~ModulationSource();

  /** Should be overriden by subclasses to update the "modValue" member variable per sample. It 
  should assign modValue to the output signal value of the modulator. */
  virtual void updateModulationValue() = 0;
    // todo: replace by a double getModulatorOutputSample()

  /** Sets up a name for this ModulationSource. This should be unique among all the available 
  ModulationSource names, so it can be used to identify the source in state recall. */
  void setModulationSourceName(const juce::String& newName) { modSourceName = newName; }

  /** Returns the name of this ModulationSource. */
  juce::String getModulationSourceName() { return modSourceName; }

protected:

  double modValue = 0;
  juce::String modSourceName = "ModulationSource";

  friend class ModulationConnection;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationSource)
};

//=================================================================================================

/** Baseclass for modulation targets.  */

class JUCE_API ModulationTarget : public ModulationParticipant
{

public:

  /** Constructor */
  ModulationTarget(ModulationManager* managerToUse = nullptr) 
    : ModulationParticipant(managerToUse) {}

  /** Destructor */
  virtual ~ModulationTarget();

  /** Must be overriden by subclasses to do whatever they need to do after our modulatedValue has 
  been computed (for example, ModulatableParameter invokes the setter-callback which in turn 
  updates the corresponding value in the core dsp algorithm). */
  virtual void doModulationUpdate() = 0;


  /** \name Setup */

  /** Sets the nominal, unmodulated value. This will be used as reference, when a modulated value 
  will be computed. */
  void setUnmodulatedValue(double newValue)
  {
    unmodulatedValue = newValue;
  }

  /** Adds a ModulationSource to this ModulationTarget. The amount of modulation is initially 0. */
  void addModulationSource(ModulationSource* source);

  /** Removes a ModulationSource from this ModulationTarget. */
  void removeModulationSource(ModulationSource* source);

  /** Sets the minimum value of the allowed modulation range. These min/max values for the range 
  are used to clip the final modulated value (after all modulations have been applied) to a sane 
  range. For example, a cutoff frequency of a filter should perhaps be clipped at 0 and 
  sampleRate/2, otherwise the filter may go crazy. */
  inline void setModulationRangeMin(double newMin) { rangeMin = newMin; }

  /** Sets the maximum value of the allowed modulation range.  */
  inline void setModulationRangeMax(double newMax) { rangeMax = newMax; }


  /** \name Inquiry */

  /** Returns the minimum value of the allowed modulation range. */
  inline double getModulationRangeMin() { return rangeMin; }

  /** Returns the maximum value of the allowed modulation range. */
  inline double getModulationRangeMax() { return rangeMax; }

  /** Returns true, if there's a connection between this ModulationTarget and the given 
  ModulationSource. */
  bool isConnectedTo(ModulationSource* source);

  /** Returns a vector of pointers to the ModulationSources which are connected to this 
  ModulationTarget. */
  std::vector<ModulationSource*> getConnectedSources();

  /** Returns a vector of pointers to the available ModulationSources which are not connected to 
  this ModulationTarget. */
  std::vector<ModulationSource*> getDisconnectedSources();

  /** Returns a vector of pointers to ModulationConnections that are incoming into this 
  ModulationTarget. */
  std::vector<ModulationConnection*> getConnections();

  /** This function must be overriden by subclasses to return a unique name that can be used to 
  identify the target in state recall. */
  virtual juce::String getModulationTargetName() = 0;


  /** \name Misc */

  /** Initializes the modulated value by setting it to the unmodulated value. */
  inline void initModulatedValue()
  {
    modulatedValue = unmodulatedValue;
  }

  /** Function to retrieve the modulated value after all modulations have been applied. This may 
  also include a clipping function, such that the returned value is restricted to some allowable
  range. */
  inline double getModulatedValue()
  {
    return clip(modulatedValue, rangeMin, rangeMax);
  }


protected:

  double unmodulatedValue = 0;
  double modulatedValue = 0;
  double rangeMin = -INF, rangeMax = INF; 

  friend class ModulationConnection;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationTarget)
};

//=================================================================================================

/** This class represents a connection from a ModulationSource to a ModulationTarget with 
adjustable modulation amount. */

class JUCE_API ModulationConnection
{

public:

  /** Constructor. You should pass a ModulationSource and a ModulationTarget. If you want to enable
  meta-control for the modulation-depth associated with the connection, you may pass the pointer to
  the MetaParameterManager object that should be used for this. */
  ModulationConnection(ModulationSource* source, ModulationTarget* target, 
    MetaParameterManager* metaManager);
    // maybe disallow metaManager to be a nullptr - or handle the case with a null object in 
    // MetaControlledParameter

  /** Destructor. */
  virtual ~ModulationConnection(); // maybe make non-virtual

  /** Sets the modulation depth for this connection. This will acually update the Parameter object 
  that is associated with the depth. */
  void setDepth(double newDepth)
  {
    depthParam->setValue(newDepth, true, true);
  }

  /** Sets the parameter range and value of the modulation depths. */
  void setDepthRangeAndValue(double newMin, double newMax, double newValue)
  {
    depthParam->setRangeAndValue(newMin, newMax, newValue, true, true);
  }

  /** Sets the parameter to relative mode in which case the output signal of the ModulationSource
  will be multiplied by the ModulationTarget's unmodulated nominal value before being added to the 
  target value. */
  inline void setRelative(bool shouldBeRelative)
  {
    relative = shouldBeRelative;
  }

  inline bool isRelative()
  {
    return relative;
  }

  /** Returns the Parameter object that controls the amount of modulation */
  MetaControlledParameter* getDepthParameter() { return depthParam; }

  /** Applies the source-value to the target-value with given amount. */
  inline void apply()
  {
    // optimize away later:
    double scaler = 1; // make this a member
    if(relative)
      scaler = target->unmodulatedValue;

    *targetValue += *sourceValue * depth * scaler; // only this line shall remain after optimization
  }


  /** Returns a xml element containing the information about this connection (needed for state 
  save/recall in ModulationManager - todo: maybe move it to there) */
  XmlElement* getAsXml();


protected:

  /** Sets the modulation depth for this connection. Used a target callback in the depthParam. To 
  keep our "depth" member consistent with the value of the "depthParam" Parameter object, outside
  code should use setDepth.
  Should not be used in state-recall because then, the depthParam won't be updated. 
  ToDo: rename this to setDepthInternal (or something) and provide a setDepth implementation that 
  *can* be used in state-recall. */
  void setDepthMember(double newDepth)
  {
    depth = newDepth;
  }


  ModulationSource* source;
  ModulationTarget* target;
  double* sourceValue;
  double* targetValue;
  double depth;
  bool relative;

  MetaControlledParameter* depthParam; // maybe it should be a ModulatableParameter? but that may
                                        // raise some issues - maybe later...

  friend class ModulationTarget;
  friend class ModulationManager;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationConnection) 
};

//=================================================================================================

/** This class is responsible for keeping a list of all available ModulationSources and 
ModulationTargets and making it possible for ModulationTargets to connect themselves to any number 
of these sources. It's also responsible for computing all modulated values and updating the targets
accordingly. */

class JUCE_API ModulationManager
{

public:

  /** Constructor. You must pass a CriticalSection object that will be used to access our arrays of
  sources, targets and connections (they are accessed from the gui and audio-thread, so the 
  accesses have to mutexed). */
  ModulationManager(CriticalSection *lockToUse);

  /** Destructor */
  virtual ~ModulationManager();

  /** Function to do the per-sample updates of all modulation-sources and targets. Should be called
  from outside code once per sample before the per-sample functions of the actual dsp-algorithms 
  (oscs, filters, whatever) are called. */
  void applyModulations();

  /** Same as applyModulations() but it doesn't acquire the mutex-lock. This should be used in cases 
  where the caller already has acquired the lock (i.e. it already holds the lock for the same 
  CriticalSection object that was initially passed to our constructor). */
  void applyModulationsNoLock();


  /** \name Connection setup */

  /** Adds a connection between the given source and target. */
  void addConnection(ModulationSource* source, ModulationTarget* target);

  /** Adds the passed ModulationConnection to our array. */
  void addConnection(ModulationConnection* connection);

  /** Removes a connection between the given source and target. */
  void removeConnection(ModulationSource* source, ModulationTarget* target);

  /** Removes all modulation connections that involve the given source. */
  void removeConnectionsWith(ModulationSource* source);

  /** Removes all modulation connections that involve the given target. */
  void removeConnectionsWith(ModulationTarget* target);

  /** Removes all ModulationConnections. */
  void removeAllConnections();

  /** Resets all the range limits for all registered modulation targets to +-inf. */
  void resetAllTargetRangeLimits();


  /** \name Registration of sources and targets */

  /** Registers the given ModulationSource to make it available to ModulationTargets. */
  void registerModulationSource(ModulationSource* source);

  /** De-registers a ModulationSource. */
  void deRegisterModulationSource(ModulationSource* source);

  /** De-registers all ModulationSources. */
  void deRegisterAllSources();

  /** Registers the given ModulationTarget. */
  void registerModulationTarget(ModulationTarget* target);

  /** De-registers a ModulationTarget. */
  void deRegisterModulationTarget(ModulationTarget* target);

  /** De-registers all ModulationTargets. */
  void deRegisterAllTargets();


  /** \name Inquiry */

  /** Returns true if there's a connection between the given source and target. */
  bool isConnected(ModulationSource* source, ModulationTarget* target);

  /** Returns the number of ModulationConnections. */
  inline int getNumConnections() 
  { 
    ScopedLock scopedLock(*modLock); 
    return size(modulationConnections); 
  }

  /** Returns a reference to our vector of available ModulationSources. */
  const std::vector<ModulationSource*>& getAvailableModulationSources() 
  { 
    ScopedLock scopedLock(*modLock); 
    return availableSources; 
  }

  /** Returns a reference to our vector of available ModulationTargets. */
  const std::vector<ModulationTarget*>& getAvailableModulationTargets() 
  { 
    ScopedLock scopedLock(*modLock); 
    return availableTargets; 
  }

  /** Returns a reference to our vector of ModulationConnections. */
  const std::vector<ModulationConnection*>& getModulationConnections()
  { 
    ScopedLock scopedLock(*modLock); 
    return modulationConnections; 
  }

  /** Given a pointer to a ModulationSource of some type, this function returns the number of 
  ModulationSources of the same type that are registered here. This is used to figure out, for 
  example, how many LFOs, envelopes, etc. already exist in order to assign an appropriate name
  to the next one to be added (it's actually not currently used). */
  int numRegisteredSourcesOfType(ModulationSource* source);

  /** Returns a pointer to the ModulationSource with given name, if a source with that name exists 
  in our array of registered sources. Otherwise, it will return a nullptr. */
  ModulationSource* getSourceByName(const juce::String& sourceName);

  /** Returns a pointer to the ModulationTarget with given name, if a source with that name exists 
  in our array of registered targets. Otherwise, it will return a nullptr. */
  ModulationTarget* getTargetByName(const juce::String& targetName);

  /** Returns true, if any of our affected targets has some range limits set up. If so, we need to
  store them in a dedicated xml-element when producing a state xml. */
  bool needsToStoreRangeLimits();


  /** \name Misc */

  /** Sets the MetaParameterManager that should be used to attach meta-parameters to the 
  depth-parameters of the modulation connections. */
  virtual void setMetaParameterManager(MetaParameterManager* managerToUse);

  /** Recalls a state (i.e. all the connections and their settings) from an XmlElement. */
  virtual void setStateFromXml(const XmlElement& xmlState);

  /** Returns the state (i.e. all the connections and their settings) in form of an XmlElement. */
  virtual XmlElement* getStateAsXml();

  /** Updates our affectedTargets array. Called from the connection-removal functions in order to 
  remove the target from the affectedTargets, in case it has no incoming connections anymore after 
  connection removal. */
  void updateAffectedTargetsArray();

protected:

  std::vector<ModulationSource*> availableSources;
  std::vector<ModulationTarget*> availableTargets;
  std::vector<ModulationTarget*> affectedTargets;
  std::vector<ModulationConnection*> modulationConnections;

  CriticalSection *modLock = nullptr; 
  MetaParameterManager* metaManager = nullptr; // use a null object instead

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationManager)
};

//=================================================================================================

class AudioModule; // ModulatableParameter needs a pointer to its owning AudioModule

/** A subclass of Parameter that is suitable as target for modulations by also being a subclass of
ModulationTarget. */

class JUCE_API ModulatableParameter : public MetaControlledParameter, public ModulationTarget
{

public:

  /** Constructor */
  ModulatableParameter(const juce::String& name, double min = 0.0, double max = 1.0,
    double defaultValue = 0.5, int scaling = LINEAR, double interval = 0.0)
    : MetaControlledParameter(name, min, max, defaultValue, scaling, interval) {}

  /** Overriden in order to also set up unmodulatedValue member inherited from ModulationTarget. */
  virtual void setValue(double newValue, bool sendNotification, bool callCallbacks) override
  {
    MetaControlledParameter::setValue(newValue, sendNotification, callCallbacks);
    ModulationTarget::setUnmodulatedValue(newValue);
  }

  /** Sets up the pointer to our owner, i.e. the AudioModule that contains this parameter (needed 
  for unique identification of this parameter in the tree of AudioModules when a state is 
  recalled). */
  void setOwnerAudioModule(AudioModule *newOwner) { ownerModule = newOwner; }

  /** We suppose that modulatedValue of the ModulationTarget baseclass has already been computed, 
  such that modulatedValue has a legitimate value with all modulations applied. Here we pull out 
  this modulated value and call our valueChangeCallback with it. The function is supposed to be 
  called per sample for each modulated Parameter. ModulationManager will take care of this. */
  inline void callCallbackWithModulatedValue()
  {
    if( valueChangeCallbackDouble != nullptr )
      valueChangeCallbackDouble->call(getModulatedValue());
  }

  /** Overriden to call our callback function with the modulated value. */
  virtual void doModulationUpdate() override
  {
    callCallbackWithModulatedValue();
  }

  /** Returns the unique name of this modulation target that is used to identify it in state 
  recall. */
  virtual juce::String getModulationTargetName() override;

protected:


  AudioModule* ownerModule = nullptr; // needed to create the unique name for state recall
  // maybe we could use the ParameterManager baseclass of AudioModule here

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulatableParameter)
};

/** This class is like ModulatebleParameter except that it uses the other std::function based
callback, so if you use this callbakc mechanism, use thsi class for your parameters */ 
class JUCE_API ModulatableParameter2 : public ModulatableParameter
{
  using ModulatableParameter::ModulatableParameter; // import baseclass constructors
  virtual void doModulationUpdate() override
  {
    valueChangeCallbackFunction(getModulatedValue());
  }
};




#endif