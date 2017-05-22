#ifndef EVENTDIAGRAMLAYER_H
#define EVENTDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class EventDiagramLayer : public GLHeapDiagramLayer
{
public:
  EventDiagramLayer();
  void debugDumpVertexTransformation();
};

#endif // EVENTDIAGRAMLAYER_H
