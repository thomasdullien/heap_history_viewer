#ifndef ACTIVEPAGESDIAGRAMLAYER_H
#define ACTIVEPAGESDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class ActivePagesDiagramLayer : public GLHeapDiagramLayer {
public:
  ActivePagesDiagramLayer();
  std::pair<vec4, vec4> vertexShaderSimulator(const HeapVertex& vertex);
  void loadVerticesFromHeapHistory(const HeapHistory& history);
};

#endif // ACTIVEPAGESDIAGRAMLAYER_H
