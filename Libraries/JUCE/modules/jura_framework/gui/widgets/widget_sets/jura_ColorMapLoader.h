#ifndef jura_ColorMapLoader_h
#define jura_ColorMapLoader_h

/** A componenent to preview a ColorMap as a rectangle showing the color gradient from lowest
to highest value. If the component is wider than high, the gradient will be shown horizontally 
(suitable for the loader widget set), otherwise vertically (suitable for a color bar next to a 
plot). */

class JUCE_API ColorMapPreviewer : public Component
{

public:

  ColorMapPreviewer(ColorMap *mapToPreview);

  virtual void paint(Graphics& g) override;

protected:

  ColorMap *colorMap;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorMapPreviewer)
};

//=================================================================================================

/** A colormap subclass that can be set up by loading appropriate xml files via a ColorMapLoader
GUI object. */

class JUCE_API LoadableColorMap : public ColorMap, public StateFileManager
{

public:


protected:

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoadableColorMap)
};





//=================================================================================================

/** A widget set for loading and previewing a ColorMap.  
\todo: to make Load buttons work, we need a ColorMap subclass that also derives from 
StateFileManager so it can keep track of what file is loaded, etc.
*/

class JUCE_API ColorMapLoader : public StateLoadSaveWidgetSet
{

public:

  /** Constructor. You must pass a valid pointer to a ColorMap object. This is the object that 
  will be updated, when the user loads a new colormap xml file. */
  ColorMapLoader(ColorMap *mapToUpdate);

  virtual void resized() override;

protected:

  ColorMapPreviewer previewer;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorMapLoader)
};

#endif  