#ifndef ACTIVEPAGESDIAGRAMLAYER_H
#define ACTIVEPAGESDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class ActiveRegionsDiagramLayer : public GLHeapDiagramLayer {
public:
  ActiveRegionsDiagramLayer();
  std::pair<vec4, vec4> vertexShaderSimulator(const HeapVertex& vertex) override;
  void loadVerticesFromHeapHistory(const HeapHistory& history, bool all) override;
};

#endif // ACTIVEPAGESDIAGRAMLAYER_H
