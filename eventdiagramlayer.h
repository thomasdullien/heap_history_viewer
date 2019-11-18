#ifndef EVENTDIAGRAMLAYER_H
#define EVENTDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class EventDiagramLayer : public GLHeapDiagramLayer
{
public:
  EventDiagramLayer();
  virtual ~EventDiagramLayer() = default;
  std::pair<vec4, vec4> vertexShaderSimulator(const HeapVertex& vertex) override;
  void loadVerticesFromHeapHistory(const HeapHistory& history, bool all) override;
};

#endif // EVENTDIAGRAMLAYER_H
