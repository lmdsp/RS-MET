#ifndef RAPT_BREAKPOINTMODULATOR_H_INCLUDED
#define RAPT_BREAKPOINTMODULATOR_H_INCLUDED


//template<class T>
//class rsBreakpointModulatorData<T>;

/** A class to combine all the required data for one modulation breakpoint. 
\todo: make this class internal to BreakpointModulatorData
\todo: perhaps, we should use the thread-safe version from rosic instead of this unsafe one from 
       RSLib...but no - we want to take care of synchronization on a higher level */

template<class T>
class rsModBreakpoint
{

  //friend class rsBreakpointModulator;
  //friend class rsBreakpointModulatorData<T>;

public:

  /** This is an enumeration of the available breakpoint shapes. */
  enum shapes
  {
    STAIRSTEP = 0,
    LINEAR,
    SMOOTH,
    ANALOG,
    GROWING,
    SIGMOID,
    SPIKEY,
    SINE_1,
    SINE_2
  };

  /** Contructor. */
  rsModBreakpoint()
  {
    timeStamp   = 0.0;
    level       = 0.0;
    shapeAmount = 1.0;
    shape       = 1;
  }

//protected:

  // member variables:
  T timeStamp;
  T level;
  T shapeAmount;
  int    shape;

};


/**

This class defines the user parameters and some other shared data for the BreakpointModulator
class.

\todo:
-make this clas internal to BreakpointModulator
-make time-stamps and levels key- and velocity dependent (important especially to make attack
 shorter for high velocity while keeping the release as is)
-try polynomial shapes (quadratic, cubic, quartic, pentic)
-introduce a 'randomize levels' function which randomizes all the levels within a given range and
 with a given quantization interval (allow also musical intervals here)
-for tanh-shape use:
 tanh(x) = (exp(2*x)-1) / (exp(2*x)+1) instead of (exp(x)-exp(-x)) / (exp(x)+exp(-x))
 -> avoids one of the exponential envelopes, -> there's a similar formula for sinh

*/

template<class T>
class rsBreakpointModulatorData
{

public:

  T scaleFactor;
  T offset;
  T bpm;
  T sampleRate;
  T minimumAllowedLevel;
  T maximumAllowedLevel;
  T endLevel;
  T minBreakpointDistance;
  T timeScale;
  T timeScaleByKey;
  T timeScaleByVel;
  T depth;
  T depthByKey;
  T depthByVel;

  int loopStartIndex;
  int loopEndIndex;
  int numCyclesInLoop;  // ?
  int editMode;

  bool loopIsOn;
  bool syncMode;
  bool endLevelFixedAtZero;

  std::vector<rsModBreakpoint<T>> breakpoints;

  /** Constructor. */
  rsBreakpointModulatorData<T>()
  {
    scaleFactor           = 1.0;
    offset                = 0.0;
    bpm                   = 120.0;
    sampleRate            = 44100.0;
    minimumAllowedLevel   = -1000.0;
    maximumAllowedLevel   = +1000.0;
    endLevel              = 0.0;
    minBreakpointDistance = 0.0;    // 0.0 seconds
    timeScale             = 1.0;
    timeScaleByKey        = 0.0;
    timeScaleByVel        = 0.0;
    depth                 = 1.0;
    depthByKey            = 0.0;
    depthByVel            = 0.0;

    loopStartIndex        = 0;
    loopEndIndex          = 1;
    numCyclesInLoop       = 1;
    editMode              = 1; // == BreakpointModulator::EDIT_WITHOUT_SHIFT

    loopIsOn              = false;
    syncMode              = false;
    endLevelFixedAtZero   = false;

    // initialize the breakpoint-vector with an analog-like ADSR curve:
    breakpoints.clear();
    rsModBreakpoint<T> newBreakpoint;

    newBreakpoint.timeStamp   = 0.0;
    newBreakpoint.level       = 0.0;
    newBreakpoint.shape       = rsModBreakpoint<T>::ANALOG;
    newBreakpoint.shapeAmount = 1.0;
    breakpoints.push_back(newBreakpoint);

    newBreakpoint.timeStamp   = 0.5;
    newBreakpoint.level       = 1.0;
    newBreakpoint.shape       = rsModBreakpoint<T>::ANALOG;
    newBreakpoint.shapeAmount = 1.0;
    breakpoints.push_back(newBreakpoint);

    newBreakpoint.timeStamp   = 1.0;
    newBreakpoint.level       = 0.5;
    newBreakpoint.shape       = rsModBreakpoint<T>::ANALOG;
    newBreakpoint.shapeAmount = 1.0;
    breakpoints.push_back(newBreakpoint);

    newBreakpoint.timeStamp   = 2.0;
    newBreakpoint.level       = 0.5;
    newBreakpoint.shape       = rsModBreakpoint<T>::ANALOG;
    newBreakpoint.shapeAmount = 1.0;
    breakpoints.push_back(newBreakpoint);

    newBreakpoint.timeStamp   = 3.0;
    newBreakpoint.level       = 0.0;
    newBreakpoint.shape       = rsModBreakpoint<T>::ANALOG;
    newBreakpoint.shapeAmount = 1.0;
    breakpoints.push_back(newBreakpoint);

  }

};


/**

This is a class which generates a modulation trajectory based on a set of data->breakpoints.

\todo: remove this master/slave stuff (it's for handling polyphony - an aspect that should be
handled at some higher level of the code - but how do we avoid redundancy of the breakpoint
data?)

*/

template<class T>
class rsBreakpointModulator
{

public:

  /** This is an enumeration of the availabel edit-modes EDIT_WITH_SHIFT means
  that all succeeding data->breakpoints will be time-shifted when one brakpoint
  is being moved. */
  enum editModes
  {
    EDIT_WITHOUT_SHIFT = 1,
    EDIT_WITH_SHIFT
  };

  /** This enumeration lists the available loop-modes. */
  enum loopModes
  {
    NO_LOOP = 0,
    FORWARD_LOOP
  };

  //---------------------------------------------------------------------------------------------
  // construction/destruction:

  /** Constructor. */
  rsBreakpointModulator();

  /** Destructor. */
  ~rsBreakpointModulator();

  /** Copies the data (the content of all member variables) from the passed source into this
  instance of BreakpointModulator. */
  void copyDataFrom(const rsBreakpointModulator& source);

  //---------------------------------------------------------------------------------------------
  // parameter settings:

  /** Sets the sample-rate. */
  void setSampleRate(T newSampleRate);

  /** Sets an overall scale factor by which the output signal will be scaled. */
  void setScaleFactor(T newScaleFactor);

  /** Sets an overall offset by which the output signal will be shifted. */
  void setOffset(T newOffset);

  /** Sets a minimum for the level-values */
  void setMinimumAllowedLevel(T newMinimium);

  /** Sets a maximum for the level-values */
  void setMaximumAllowedLevel(T newMaximium);

  /** Fixes the end level of this generator at zero - useful for amplitude envelopes. */
  void fixEndLevelAtZero(bool shouldBeFixed);

  /** Inserts a new breakpoint into the vector. The new breakpoint must satisfy some
  preconditions in order to be successfully inserted (it is not allowed to be too close to an
  already existing breakpoint, for example). The integer return-value informs, at which index if
  the new breakpoint was inserted. It will return -1, when the breakpoint could not be
  inserted. */
  int  insertBreakpoint(T newTimeStamp, T newLevel, int newShape = 0,
    T newShapeAmount = 0.0);

  /** Removes a breakpoint from the vector. The return-value informs, if there was actually a
  breakpoint removed (if you try to remove a non-existing breakpoint, or a breakpoint which
  cannot be removed, it will return false */
  bool removeBreakpoint(int index);

  /** Modifies the data of an existing breakpoint. If you try to modify a non-existent breakpoint
  or try to modify the data in such a way which is not allowed, it will return false. */
  bool modifyBreakpoint(int index, T newTimeStamp, T newLevel,
    int newShape = 0, T newShapeAmount = 0.0);

  /** Changes the time (in seconds or beats) of one breakpoint and reports about the success of
  that operation. */
  bool setBreakpointTime(int index, T newTime);

  /** Changes the level of one breakpoint and reports about the success of that operation. */
  bool setBreakpointLevel(int index, T newLevel);

  /** Changes the shape (index) of one breakpoint and reports about the success of that
  operation. */
  bool setBreakpointShape(int index, int newShape);

  /** Changes the shape for all breakpoints at once. */
  void setAllBreakpointsShape(int newShape);

  /** Changes the shape-amount of one breakpoint and reports about the success of that
  operation. */
  bool setBreakpointShapeAmount(int index, T newShapeAmount);

  /** Changes the shape amount for all breakpoints at once. */
  void setAllBreakpointsShapeAmount(T newAmount);

  /** Turns looping on or off. */
  void setLoopMode(bool shouldBeLooped);

  /** Sets the start index for a loop - can be used for a generalized 'sustain'-phase. The
  loop-start index must be at least 0 and at most the numberOfBreakpoints-1 and smaller than
  or equal to the loop-end index. */
  bool setLoopStartIndex(int newLoopStartIndex);

  /** Sets the end index for a loop - can be used for a generalized 'sustain'-phase. The loop-end
  index must be at least 0 and at most the numberOfBreakpoints-1 and greater than or equal to the
  loop-start index. */
  bool setLoopEndIndex(int newLoopEndIndex);

  /** Chooses the edit-mode which is the behaviour when a breakpoint is being dragged to a new
  time-position. When editModes::EDIT_WITHOUT_SHIFT is passed, other data->breakpoints will be
  unaffected, if editModes::EDIT_WITH_SHIFT is passed, data->breakpoints right to the one which
  is being modified will shift in time also. */
  void setEditMode(int newEditMode);

  /** Sets the number number of cycles which the loop represents. This is needed for the
  calculation of the readout-frequency. */
  void setNumCyclesInLoop(int newNumberOfCyclesInLoop);

  /** Switches into sync mode. This will have the effect that all time-values will be interpreted
  as whole notes instead of seconds. The absolute time-values will be calculated from these
  beat-values according to the bpm-variable which can be adjusted via setBeatsPerMinute. */
  void setSyncMode(bool shouldBeSynced);

  /** Sets the beats per minute value in order to allow beat syncronous modulations (call
  setSyncMode() to toggle sync-mode on or off). */
  void setBeatsPerMinute(T newBpm);

  /** Sets the time scale factor. */
  void setTimeScale(T newTimeScale);

  /** Sets the key dependence of the time scale factor in percent. */
  void setTimeScaleByKey(T newTimeScaleByKey);

  /** Sets the velocity dependence of the time scale factor. */
  void setTimeScaleByVel(T newTimeScaleByVel);

  /** Sets the depth. */
  void setDepth(T newDepth);

  /** Sets the key dependence of the depth. */
  void setDepthByKey(T newDepthByKey);

  /** Sets the velocity dependence of the depth. */
  void setDepthByVel(T newDepthByVel);

  //---------------------------------------------------------------------------------------------
  // inquiry:

  /** Returns the index of the last breakpoint (the end-point). */
  int lastBreakpointIndex() const;  // \todo: return rsUint32

  /** Returns the number of data->breakpoints. */
  int getNumBreakpoints() const;  // \todo: return rsUint32

  /** Returns the overall scaling factor of the modulation signal. */
  T getScaleFactor() const;

  /** Returns the overall shift/offset factor of the modulation signal. */
  T getOffset() const;

  /** Returns the starting time (in seconds or beats), is usually zero. */
  T getStartTime() const;

  /** Returns the end time (in seconds or beats), i.e. the time stamp of thelast breakpoint. */
  T getEndTime() const;

  /** Returns the minimum level of the curve. */
  T getMinLevel() const;

  /** Returns the maximum level of the curve. */
  T getMaxLevel() const;

  /** Returns the absolute time (in seconds or beats) of one breakpoint. If the index is out of
  range, it will return -1.0. */
  T getBreakpointTime(int index) const;

  /** Returns the minimum value to which the time variable of a breakpoint can be set without
  violating the constraints to not come too close to its neighbours. If the index is out of
  range, it will return -1.0. */
  T getBreakpointMinTime(int index) const;

  /** Returns the maximum value to which the time variable of a breakpoint can be set without
  violating the constraints to not come too close to its neighbours. If the index is out of
  range, it will return -1.0. */
  T getBreakpointMaxTime(int index) const;

  /** Returns the level of one breakpoint. If the index is out of range, it will return 0.0. */
  T getBreakpointLevel(int index) const;

  /** Returns the (index of) the shape of one breakpoint. If the index is out of range, it will
  return -1. */
  int getBreakpointShape(int index) const;

  /** Returns the amount of the shape of one breakpoint. If the index is out of range, it will
  return 0.0. */
  T getBreakpointShapeAmount(int index) const;

  /** Returns the current loop mode. */
  int getLoopMode() const;

  /** Returns the index of the breakpoint at which the loop starts. */
  int getLoopStartIndex() const;

  /** Returns the index of the breakpoint at which the loop ends. */
  int getLoopEndIndex() const;

  /** Returns the number of cycles contained in the loop. */
  int getNumCyclesInLoop() const;

  /** Returns true, when sync is toggled on, false otherwise. */
  bool isInSyncMode() const;

  /** Returns the number of cycles per time unit (seconds or whole notes). */
  //T getNumCyclesPerTimeUnit() const;

  /** Returns the current tempo to which this object is set. */
  T getBeatsPerMinute() const;

  /** Returns the time scale factor. */
  T getTimeScale() const;

  /** Returns the key dependence of the time scale factor. */
  T getTimeScaleByKey() const;

  /** Returns the velocity dependence of the time scale factor. */
  T getTimeScaleByVel() const;

  /** Returns the time scale factor. */
  T getDepth() const;

  /** Returns the key dependence of the time scale factor. */
  T getDepthByKey() const;

  /** Returns the velocity dependence of the time scale factor. */
  T getDepthByVel() const;

  //---------------------------------------------------------------------------------------------
  // event-handling:

  /** Causes the envelope to start with its attack-phase. When the parameter
  "startFromCurrentLevel" is true, the internal state will not be reset to the start-level, such
  that the curve begins at the level, where the envelope currently is. */
  void noteOn(bool startFromCurrentLevel = false, int newKey = 64, int newVel = 64);

  /** This simulates the condition, that a note is on and already held for some amount of time
  (expressed in units of samples). It can be used in order to render only a later part of
  the modulation signal, leaving out its beginning - this is useful for displaying parts of
  the curve in a zoomed in GUI editor. */
  void noteOnAndAdvanceTime(int sampleIndexToStartFrom = 0);

  /** Causes the envelope to start with its release-phase. */
  void noteOff(bool startFromCurrentLevel = true);

  /** Sets everything except the sampleRate to default-values. */
  void setToDefaultValues();

  /** Sets the object to an initial state with only two data->breakpoints at times 0.0 and 1.0
  and with values 0.0 both. ...is this still true? */
  void initialize();

  //---------------------------------------------------------------------------------------------
  // audio processing:

  /** Calculates one output sample at a time. */
  inline T getSample();

  /** Fills the passed buffer with the envelope (or the loop-portion thereof). This function
  comes in handy for generating waveforms for the LowFrequencyOscillator class. */
  void fillBufferWithEnvelope(T *buffer, int length, bool useOnlyLoop);

  //---------------------------------------------------------------------------------------------
  // master/slave configuration:

  /** Adds a slave instance to the vector of slaves - this will also cause the 'isMaster' flag
  of the slave to be set to false, redirect the slaves parameters-pointer to the one of this
  instance and delete the old (now unused) parameters pointer of the slave. */
  void addSlave(rsBreakpointModulator* newSlave);

  //---------------------------------------------------------------------------------------------
  // public data members:

  /** Flag to indicate, if the modulator has reached its end-point. */
  bool endIsReached;

  //=============================================================================================

protected:

  /** Does all the necesarry stuff when a breakpoint is reached, in particular, it sets up all
  the variables that are used in the getSample() function. */
  void handleBreakpointArrival();

  /** Does the wraparound for 'leftIndex' and 'rightIndex' back to the loop-start. */
  void wrapAroundIndicesForLoop();

  /** Returns the index of the next breakpoint which has a timeStamp strictly larger then the
  timeStamp of the startIndex - mostly this will be just one number higher than startIndex, but
  when there are more breakpoints at one time instant, it will be higher than that. If the result
  is out of range, it will return 0. */
  int getNextNonSimultaneousIndex(int startIndex);

  /** Return the last index at the same timeStamp as startIndex - if there is only one breakpoint
  at this timeStamp, this will return the startIndex iteself. */
  int getLastSimultaneousIndex(int startIndex);

  /** This function calculates and initializes the required state-variables for the recursions.
  The optional parameter indicates at which sample (relative to the segment) we want to start
  the recursion - this allows to avoid redering the whole segment when actually only part of
  it is needed (such as for GUI displays that are zoomed in). */
  void setupStateVariables();

  /** Updates the 'timeScaleFactor' member variable according to 'data->timeScale',
  'data->timeScaleByKey',  data->timeScaleByVel, 'currentKey' and 'currentVel' */
  void updateTimeScaleFactor();

  /** Updates the 'samplesToNextBreakpoint' member variable according to 'leftIndex',
  'rightIndex', 'timeScaleFactor' and possibly (data->syncMode and data->bpm). */
  void updateSamplesToNextBreakpoint();

  /** Returns a number which is clipped into the range between minimumAllowedLevel and
  maximumAllowedLevel. */
  T clipLevelToRange(T inLevel);

  /** Returns the level at a given index scaled by the current key and velocity. */
  T scaleLevelByKeyAndVelocity(T unscaledLevel);

  /** Make a copy-constructor unavailable because this class needs deep copying (because of the
  pointers in the MutexLocks). If you need to create a new object based on an existing one,
  simply create a new object with the default constructor and then use copyDataFrom(). 
  ...doesn't apply anymore...  */
  //rsBreakpointModulator(const rsBreakpointModulator& modulatorToCopy);

  //---------------------------------------------------------------------------------------------

  /** Buffering-variables for the variuos recursions. */
  T state1, state2;

  /** The increment/decrement for state1 and state2 (which can have multiplictive or additive
  nature depending on the chosen shape, so we call it genrally 'change'). */
  T state1_change, state2_change;

  /** Level of the breakpoint left to (before) or at the current time, scaled by key and
  velocity. */
  T leftLevel;

  /** Level of the breakpoint right to (after) or at the current time, scaled by key and
  velocity. */
  T rightLevel;

  /** The difference between the levels of the two data->breakpoints to the left
  and to the right. */
  T levelDelta;

  /** The minimum-values for state1 and state2. */
  T state1_min, state2_min;

  /** The maximum-values for state1 and state2. */
  T state1_max, state2_max;

  /** Some scale-factors, to scale the normalized modulation shape (which is
  created in the range 0...1) to the desired range. */
  T scaler1, scaler2;

  /** The shape of the breakpoint, we are currently aiming. */
  int currentShape;

  /** A counter which counts down to the next breakpoint. When the counter reaches zero, the
  envelope has reached a new breakpoint. */
  int samplesToNextBreakpoint;

  /** Because the time difference between two successive breakpoints must be rounded to an
  integer number of samples, a timing error accumulates during the course of the envelope. This
  variable keeps track of this error and whenever it exceed +- 0.5 samples, an extra sample is
  inserted or a sample is dropped. */
  T accumulatedTimingError;

  /** Index of the breakpoint from which we come, this is  to the left on the time-axis (or we
  sit exactly on it). */
  int leftIndex;

  /** Index of the breakpoint to which we go. */
  int rightIndex;

  /** Indicates if note is on, if not, the envelope will not trap into the loop. */
  bool noteIsOn;

  /** Value of the previous output sample. This variable is also used to represent a constant
  output level (at the end and in sustain-loops of length zero). */
  T previousOut;

  /** Scales the overall time-length of the modulator. */
  T timeScaleFactor;

  /** Current note key and velocity. */
  int currentKey, currentVel;

  /** A pointer to the data which are potentially shared by among instances. */
  rsBreakpointModulatorData<T>* data;

  /** A vector of pointers to other instances of this class which shall be kept in sync to this
  instance with regard to their parameters. */
  std::vector<rsBreakpointModulator*> slaves;

  /** A flag which indicates whether or not this instance is a master which controls other
  instances of this class - this will also determine whether or not this objects will delete the
  pointer to the parameter set on destruction. By default, instances will be constructed as
  master, later they can be re-configured as slaves by adding them as slaves via addSlave to
  another instance. */
  bool isMaster;

  /** Flag to indicate whether the output level is constant (happens when the end is reached and
  fo sustain-loops with zero length. The 'previousOut' member will be used in this case to
  represent that level. */
  bool outLevelIsConstant;

};

//-----------------------------------------------------------------------------------------------
// from here: definitions of the functions to be inlined, i.e. all functions which are supposed
// to be called at audio-rate (they can't be put into the .cpp file):

template<class T>
inline T rsBreakpointModulator<T>::getSample()
{
  T tmp1, tmp2; // temporary variables for intermediate results
  T out = 0.0;  // output variable

  // return the value of the 'previousOut' member when the output is constant:
  if(outLevelIsConstant)
    return previousOut;

  // do the necessary initializations for the various variables when we are at a breakpoint:
  if(samplesToNextBreakpoint <= 0)
  {
    handleBreakpointArrival();
    if(outLevelIsConstant)
      return previousOut;
  }

  // do the specific per-sample calculations for the different envelope shapes (for details about
  // what's going on, refer to the comments in the MatLab/Octave implementation):
  switch(currentShape)
  {
  case rsModBreakpoint<T>::STAIRSTEP:
  {
    out     = state1;
  }
  break;
  case rsModBreakpoint<T>::LINEAR:
  {
    out     = state1;
    state1 += state1_change;
  }
  break;
  case rsModBreakpoint<T>::SMOOTH:
  {
    tmp1   = state1_change*state1 - state2;
    state2 = state1;
    state1 = tmp1;
    out    = leftLevel + levelDelta*(0.5f*tmp1+0.5f);
  }
  break;
  case rsModBreakpoint<T>::ANALOG:
  {
    out     = leftLevel + levelDelta - scaler1*(state1-state1_min);
    state1 *= state1_change;
  }
  break;
  case rsModBreakpoint<T>::GROWING:
  {
    out     = leftLevel + scaler1*(state1-state1_min);
    state1 *= state1_change;
  }
  break;
  case rsModBreakpoint<T>::SIGMOID:
  {
    tmp1 = scaler1*(state1-state1_min);
    tmp2 = scaler1*(state2-state1_min); // state2_min == state1_min
    if(tmp1+tmp2 == 0.0)
      tmp1 = 0.0;
    else
      tmp1 = (tmp2-tmp1) / (tmp2+tmp1);
    tmp1    = 0.5f * tmp1 + 0.5f; // may be optimized away later
    out     = leftLevel + levelDelta*tmp1;
    state1 *= state1_change;
    state2 *= state2_change;
  }
  break;
  case rsModBreakpoint<T>::SPIKEY:
  {
    tmp1    = scaler1*(state1-state1_min);
    tmp2    = scaler1*(state2-state1_min); // state2_min == state1_min
    tmp1    = 0.5f*(tmp2-tmp1) + 0.5f;
    out     = leftLevel + levelDelta*tmp1;
    state1 *= state1_change;
    state2 *= state2_change;
  }
  break;
  case rsModBreakpoint<T>::SINE_1:
  {
    tmp1   = state1_change*state1 - state2;
    state2 = state1;
    state1 = tmp1;
    out    = leftLevel + levelDelta*(tmp1);
  }
  break;
  case rsModBreakpoint<T>::SINE_2:
  {
    tmp1   = state1_change*state1 - state2;
    state2 = state1;
    state1 = tmp1;
    out    = leftLevel + 2*levelDelta*(0.5f*tmp1+0.5f);
  }
  break;

  default:
  {
    out     = state1;
    state1 += state1_change;
  }
  } // end of switch( data->breakpoints[rightIndex].shape )

  // decrement countdown to next breakpoint:
  samplesToNextBreakpoint--;

  // remember the output for the next call (might be needed, if we enter the
  // release-phase (the segments(s) behind the loop) due to a note-off):
  previousOut = out;

  // and return the generated output:
  return out;
}

#endif
