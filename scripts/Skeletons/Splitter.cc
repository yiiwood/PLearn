#include "DERIVEDCLASS.h"

namespace PLearn <%
using namespace std;

DERIVEDCLASS::DERIVEDCLASS() 
  :Splitter()
  /* ### Initialise all fields to their default value */
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT_METHODS(DERIVEDCLASS, "DERIVEDCLASS", Splitter);

void DERIVEDCLASS::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  // declareOption(ol, "myoption", &DERIVEDCLASS::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...

  // Now call the parent class' declareOptions
  parentclass::declareOptions(ol);
}

string DERIVEDCLASS::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "DERIVEDCLASS implements a ..."
    + optionHelp();
}

void DERIVEDCLASS::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void DERIVEDCLASS::build()
{
  parentclass::build();
  build_();
}

void DERIVEDCLASS::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Splitter::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int DERIVEDCLASS::nsplits() const
{
  // ### Return the number of available splits
}

int DERIVEDCLASS::nSetsPerSplit() const
{
  // ### Return the number of sets per split
}

TVec<VMat> DERIVEDCLASS::getSplit(int k)
{
  // ### Build and return the kth split 
}


%> // end of namespace PLearn
