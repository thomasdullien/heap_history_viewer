#ifndef EVENTDIAGRAMLAYER_H
#define EVENTDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class EventDiagramLayer : public GLHeapDiagramLayer
{
public:
  EventDiagramLayer();
  std::pair<vec4, vec4> vertexShaderSimulator(const HeapVertex& vertex);
  void loadVerticesFromHeapHistory(const HeapHistory& history);
};

#endif // EVENTDIAGRAMLAYER_H
