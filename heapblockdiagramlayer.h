#ifndef HEAPBLOCKDIAGRAMLAYER_H
#define HEAPBLOCKDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class HeapBlockDiagramLayer : public GLHeapDiagramLayer {
public:
  HeapBlockDiagramLayer();
  std::pair<vec4, vec4> vertexShaderSimulator(const HeapVertex& vertex);
  void loadVerticesFromHeapHistory(const HeapHistory& history);
};

#endif // HEAPBLOCKDIAGRAMLAYER_H
