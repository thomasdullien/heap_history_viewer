#ifndef ADDRESSDIAGRAMLAYER_H
#define ADDRESSDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class AddressDiagramLayer : public GLHeapDiagramLayer {
public:
  AddressDiagramLayer();
  std::pair<vec4, vec4> vertexShaderSimulator(const HeapVertex& vertex);
};

#endif // ADDRESSDIAGRAMLAYER_H
