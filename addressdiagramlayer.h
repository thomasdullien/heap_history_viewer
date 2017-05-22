#ifndef ADDRESSDIAGRAMLAYER_H
#define ADDRESSDIAGRAMLAYER_H
#include "glheapdiagramlayer.h"

class AddressDiagramLayer : public GLHeapDiagramLayer {
public:
  AddressDiagramLayer();
  void debugDumpVertexTransformation();
};

#endif // ADDRESSDIAGRAMLAYER_H
