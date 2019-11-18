#ifndef HEAPBLOCKDIAGRAMLAYER_H
#define HEAPBLOCKDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class HeapBlockDiagramLayer : public GLHeapDiagramLayer {
public:
  HeapBlockDiagramLayer();
  virtual ~HeapBlockDiagramLayer() = default;
  std::pair<vec4, vec4> vertexShaderSimulator(const HeapVertex& vertex) override;
  void loadVerticesFromHeapHistory(const HeapHistory& history, bool all) override;
};

#endif // HEAPBLOCKDIAGRAMLAYER_H
