
std::vector<ModulationSource*> ModulationParticipant::dummySources;
std::vector<ModulationTarget*> ModulationParticipant::dummyTargets;
std::vector<ModulationConnection*> ModulationParticipant::dummyConnections;

const std::vector<ModulationSource*>& ModulationParticipant::getAvailableModulationSources()
{
  if(modManager)
    return modManager->getAvailableModulationSources();
  else
    return dummySources;
}

const std::vector<ModulationTarget*>& ModulationParticipant::getAvailableModulationTargets()
{
  if(modManager)
    return modManager->getAvailableModulationTargets();
  else
    return dummyTargets;
}

const std::vector<ModulationConnection*>& ModulationParticipant::getModulationConnections()
{
  if(modManager)
    return modManager->getModulationConnections();
  else
    return dummyConnections;
}

void ModulationParticipant::registerModulationSource(ModulationSource* source)
{
  if(modManager)
    modManager->registerModulationSource(source);
}

void ModulationParticipant::deRegisterModulationSource(ModulationSource* source)
{
  if(modManager)
    modManager->deRegisterModulationSource(source);
}

void ModulationParticipant::registerModulationTarget(ModulationTarget* target)
{
  if(modManager)
    modManager->registerModulationTarget(target);
}

void ModulationParticipant::deRegisterModulationTarget(ModulationTarget* target)
{
  if(modManager)
    modManager->deRegisterModulationTarget(target);
}

//-------------------------------------------------------------------------------------------------

ModulationSource::~ModulationSource() 
{
  ModulationParticipant::deRegisterModulationSource(this);
}

//-------------------------------------------------------------------------------------------------

ModulationTarget::~ModulationTarget() 
{
  ModulationParticipant::deRegisterModulationTarget(this);
}

void ModulationTarget::addModulationSource(ModulationSource* source)
{
  if(modManager)
    modManager->addConnection(source, this);
}

void ModulationTarget::removeModulationSource(ModulationSource* source)
{
  if(modManager)
    modManager->removeConnection(source, this);
}

bool ModulationTarget::isConnectedTo(ModulationSource* source)
{
  if(modManager)
    return modManager->isConnected(source, this);
  return false;
}

std::vector<ModulationSource*> ModulationTarget::getConnectedSources()
{
  std::vector<ModulationSource*> result;
  if(modManager)
  {
    const std::vector<ModulationSource*>& allSources = modManager->getAvailableModulationSources();
    for(int i = 0; i < size(allSources); i++)
    {
      if(this->isConnectedTo(allSources[i]))
        result.push_back(allSources[i]);
    }
  }
  return result;
}

std::vector<ModulationSource*> ModulationTarget::getDisconnectedSources()
{
  std::vector<ModulationSource*> result;
  if(modManager)
  {
    const std::vector<ModulationSource*>& allSources = modManager->getAvailableModulationSources();
    for(int i = 0; i < size(allSources); i++)
    {
      if(!this->isConnectedTo(allSources[i])) // ! is only difference to getConnectedSources, maybe
        result.push_back(allSources[i]);      // we can refactor to avoid code duplication
    }
  }
  return result;
}

std::vector<ModulationConnection*> ModulationTarget::getConnections()
{
  std::vector<ModulationConnection*> result;
  if(modManager)
  {
    const std::vector<ModulationConnection*>& 
      allConnections = modManager->getModulationConnections();
    for(int i = 0; i < size(allConnections); i++)
    {
      if(this == allConnections[i]->target)
        result.push_back(allConnections[i]);
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------

ModulationConnection::ModulationConnection(ModulationSource* _source, ModulationTarget* _target, 
  MetaParameterManager* metaManager)
{
  source   = _source; 
  target   = _target;
  depth    = 0.0;
  //relative = false;
  relative = true;
  sourceValue = &(source->modValue);
  targetValue = &(target->modulatedValue);

  juce::String name = source->getModulationSourceName();
  depthParam = new MetaControlledParameter(name, -1.0, 1.0, 0.0, Parameter::LINEAR, 0.0);
  depthParam->setValueChangeCallback<ModulationConnection>(
    this, &ModulationConnection::setDepthMember);
  depthParam->setMetaParameterManager(metaManager);
}

ModulationConnection::~ModulationConnection()
{
  delete depthParam;
}

XmlElement* ModulationConnection::getAsXml()
{
  XmlElement* xml = new XmlElement("Connection");

  xml->setAttribute("Source", source->getModulationSourceName());
  xml->setAttribute("Target", target->getModulationTargetName());

  xml->setAttribute("Depth",  depth);
  if(depthParam->getMinValue() != -1)
    xml->setAttribute("DepthMin", depthParam->getMinValue());
  if(depthParam->getMaxValue() != +1)
    xml->setAttribute("DepthMax", depthParam->getMaxValue());
  if(depthParam->getMetaParameterIndex() != -1)
    xml->setAttribute("DepthMeta", depthParam->getMetaParameterIndex());
    // later, we may also have to store the mapping function here (if any)

  juce::String modeString;
  if(relative)
    modeString = "Relative";
  else
    modeString = "Absolute";
    // we do it like this in order to be able to define more modes later - like Multiplicative
    // or whatever

  xml->setAttribute("Mode", modeString);
  return xml;

  // maybe move this function into ModulationManager as 
  // getConnectionXml(ModulationConnection *c);
}

//-------------------------------------------------------------------------------------------------

ModulationManager::ModulationManager(CriticalSection* lockToUse)
{
  modLock = lockToUse;
}

ModulationManager::~ModulationManager() 
{
  ScopedLock scopedLock(*modLock);
  removeAllConnections();
  deRegisterAllSources();
  deRegisterAllTargets();
}

void ModulationManager::applyModulations()
{
  ScopedLock scopedLock(*modLock); 
  applyModulationsNoLock();
}

void ModulationManager::applyModulationsNoLock()
{
  int i;

  // compute output signals of all modulators:
  for(i = 0; i < size(availableSources); i++)
    availableSources[i]->updateModulationValue();

  // initialize modulation target values with their unmodulated values:
  for(i = 0; i < size(affectedTargets); i++)
    affectedTargets[i]->initModulatedValue();

  // apply all modulations:
  for(i = 0; i < size(modulationConnections); i++)
    modulationConnections[i]->apply();

  // let the targets do whatever work they have to do with the modulated value (typically, 
  // call setter-callbacks):
  for(i = 0; i < size(affectedTargets); i++)
    affectedTargets[i]->doModulationUpdate();
}

void ModulationManager::addConnection(ModulationSource* source, ModulationTarget* target)
{
  ScopedLock scopedLock(*modLock); 
  jassert(!isConnected(source, target)); // there is already a connection between source and target
  modulationConnections.push_back(new ModulationConnection(source, target, metaManager));
  appendIfNotAlreadyThere(affectedTargets, target);
}

void ModulationManager::addConnection(ModulationConnection* connection)
{
  ScopedLock scopedLock(*modLock); 
  jassert(!isConnected(connection->source, connection->target)); // connection already exists
  modulationConnections.push_back(connection);
  appendIfNotAlreadyThere(affectedTargets, connection->target);
}

void ModulationManager::removeConnection(ModulationSource* source, ModulationTarget* target)
{
  ScopedLock scopedLock(*modLock); 
  jassert(isConnected(source, target)); // trying to remove no-existent connection
  for(int i = 0; i < size(modulationConnections); i++)
  {
    if(modulationConnections[i]->source == source && modulationConnections[i]->target == target)
    {
      delete modulationConnections[i];
      remove(modulationConnections, i);
    }
  }
  updateAffectedTargetsArray();
  jassert(!isConnected(source, target)); // there must have been more than one connection between
                                         // given source and target - that should not happen
}

void ModulationManager::removeConnectionsWith(ModulationSource* source)
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(modulationConnections); i++)
  {
    if(modulationConnections[i]->source == source)
    {
      delete modulationConnections[i];
      remove(modulationConnections, i);
      i--; // array was shrunken
    }
  }
  updateAffectedTargetsArray();
}

void ModulationManager::removeConnectionsWith(ModulationTarget* target)
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(modulationConnections); i++)
  {
    if(modulationConnections[i]->target == target)
    {
      delete modulationConnections[i];
      remove(modulationConnections, i);
      i--; // array was shrunken
    }
  }
  updateAffectedTargetsArray();
}

void ModulationManager::removeAllConnections()
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(modulationConnections); i++)
    delete modulationConnections[i];
  modulationConnections.clear();
  updateAffectedTargetsArray();
}

void ModulationManager::resetAllTargetRangeLimits()
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(availableTargets); i++)
  {
    availableTargets[i]->setModulationRangeMin(-INF);
    availableTargets[i]->setModulationRangeMax(+INF);
  }
}

void ModulationManager::registerModulationSource(ModulationSource* source)
{
  ScopedLock scopedLock(*modLock); 
  appendIfNotAlreadyThere(availableSources, source);
  source->setModulationManager(this);
}

void ModulationManager::deRegisterModulationSource(ModulationSource* source)
{
  ScopedLock scopedLock(*modLock); 
  jassert(contains(availableSources, source)); // source was never registered
  removeFirstOccurrence(availableSources, source);
  removeConnectionsWith(source);
  source->setModulationManager(nullptr); 
}

void ModulationManager::deRegisterAllSources()
{
  ScopedLock scopedLock(*modLock); 
  while(size(availableSources) > 0)
    deRegisterModulationSource(availableSources[size(availableSources)-1]);
}

void ModulationManager::registerModulationTarget(ModulationTarget* target)
{
  ScopedLock scopedLock(*modLock); 
  appendIfNotAlreadyThere(availableTargets, target);
  target->setModulationManager(this);
}

void ModulationManager::deRegisterModulationTarget(ModulationTarget* target)
{
  ScopedLock scopedLock(*modLock); 
  jassert(contains(availableTargets, target)); // target was never registered
  removeFirstOccurrence(availableTargets, target);
  removeFirstOccurrence(affectedTargets,  target);
  removeConnectionsWith(target);
  target->setModulationManager(nullptr);
}

void ModulationManager::deRegisterAllTargets()
{
  ScopedLock scopedLock(*modLock); 
  while(size(availableTargets) > 0)
    deRegisterModulationTarget(availableTargets[size(availableTargets)-1]);
}

bool ModulationManager::isConnected(ModulationSource* source, ModulationTarget* target)
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(modulationConnections); i++)
    if(modulationConnections[i]->source == source && modulationConnections[i]->target == target)
      return true;
  return false;
}

int ModulationManager::numRegisteredSourcesOfType(ModulationSource* source)
{
  ScopedLock scopedLock(*modLock); 
  int result = 0;
  for(int i = 0; i < size(availableSources); i++)
  {
    if(typeid(*source) == typeid(*availableSources[i]))
      result++;
  }
  return result;
}

ModulationSource* ModulationManager::getSourceByName(const juce::String& sourceName)
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(availableSources); i++)
  {
    if(availableSources[i]->getModulationSourceName() == sourceName)
      return availableSources[i];
  }
  return nullptr;
}

ModulationTarget* ModulationManager::getTargetByName(const juce::String& targetName)
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(availableTargets); i++)
  {
    if(availableTargets[i]->getModulationTargetName() == targetName)
      return availableTargets[i];
  }
  return nullptr;
}

bool ModulationManager::needsToStoreRangeLimits()
{
  ScopedLock scopedLock(*modLock); 
  for(int i = 0; i < size(affectedTargets); i++)
  {
    if(  affectedTargets[i]->getModulationRangeMin() != -INF
      || affectedTargets[i]->getModulationRangeMax() !=  INF)
      return true;
  }
  return false;
}

void ModulationManager::setMetaParameterManager(MetaParameterManager* managerToUse)
{
  ScopedLock scopedLock(*modLock); 
  metaManager = managerToUse;
  // todo: set it to the null object, in case a nullptr is passed
}

void ModulationManager::setStateFromXml(const XmlElement& xmlState)
{
  ScopedLock scopedLock(*modLock); 
  jassert(xmlState.hasTagName("Modulations"));  // not the right kind of xml element

  // recall connections:
  removeAllConnections();
  forEachXmlChildElementWithTagName(xmlState, conXml, "Connection")
  {
    ModulationSource* source = getSourceByName(conXml->getStringAttribute("Source"));
    ModulationTarget* target = getTargetByName(conXml->getStringAttribute("Target"));
    if(source != nullptr && target != nullptr)
    {
      ModulationConnection* c = new ModulationConnection(source, target, metaManager);

      c->depthParam->detachFromMetaParameter();
      int meta = conXml->getIntAttribute("DepthMeta", -1);
      if(meta >= 0)
        c->depthParam->attachToMetaParameter(meta);

      double min = conXml->getDoubleAttribute("DepthMin", -1.0);
      double max = conXml->getDoubleAttribute("DepthMax", +1.0);
      double val = conXml->getDoubleAttribute("Depth",     0.0);
      c->setDepthRangeAndValue(min, max, val);

      c->setRelative(conXml->getStringAttribute("Mode") == "Relative");
      addConnection(c);
    }
    else
      jassertfalse; // source and/or target with given name doesn't exist - patch corrupted?
  }

  // recall range limits:
  resetAllTargetRangeLimits();
  XmlElement* xmlLimits = xmlState.getChildByName("RangeLimits");
  if(xmlLimits != nullptr)
  {
    forEachXmlChildElement(*xmlLimits, targetLimitsXml)
    {
      ModulationTarget* target = getTargetByName(targetLimitsXml->getTagName());
      if(target != nullptr)
      {
        target->setModulationRangeMin(targetLimitsXml->getDoubleAttribute("Min", -INF));
        target->setModulationRangeMax(targetLimitsXml->getDoubleAttribute("Max", +INF));
      }
    }
  }
}

XmlElement* ModulationManager::getStateAsXml()
{
  ScopedLock scopedLock(*modLock); 

  if(size(modulationConnections) == 0)
    return nullptr; // "Modulations" child-element will be absent from preset file

  XmlElement* xmlState = new XmlElement("Modulations"); // maybe make the name a function parameter for consistency with the recall in Chainer

  for(int i = 0; i < size(modulationConnections); i++)
    xmlState->addChildElement(modulationConnections[i]->getAsXml());

  if(!needsToStoreRangeLimits())
    return xmlState;

  XmlElement* xmlLimits = new XmlElement("RangeLimits");
  for(int i = 0; i < size(affectedTargets); i++)
  {
    double min = affectedTargets[i]->getModulationRangeMin();
    double max = affectedTargets[i]->getModulationRangeMax();
    if(min != -INF || max != INF)
    {
      XmlElement* xmlTargetLimits = new XmlElement(affectedTargets[i]->getModulationTargetName());
      if(min != -INF) xmlTargetLimits->setAttribute("Min", min);
      if(max !=  INF) xmlTargetLimits->setAttribute("Max", max);
      xmlLimits->addChildElement(xmlTargetLimits);
    }
  }
  xmlState->addChildElement(xmlLimits);
  return xmlState;
}

void ModulationManager::updateAffectedTargetsArray()
{
  ScopedLock scopedLock(*modLock); 
  affectedTargets.clear();
  for(int i = 0; i < size(modulationConnections); i++)
    appendIfNotAlreadyThere(affectedTargets, modulationConnections[i]->target);
  // not sure, if this is the best (most efficient) way to do it
}

//-------------------------------------------------------------------------------------------------

juce::String ModulatableParameter::getModulationTargetName()
{
  if(ownerModule == nullptr)
  {
    jassertfalse; // You need to set the owner via setOwnerAudioModule, so we can use that to 
                  // generate a unique name.
    return String::empty;
  }
  else
    return ownerModule->getAudioModulePath() + getName();
}