#ifndef __incl_ISelectionProvider_h
#define __incl_ISelectionProvider_h

#include <vector>
#include "../../Math/Cameras/Camera.h"

class ISelectionProvider
{
  public:
    virtual unsigned int CountSelectable() = 0;
    virtual void RenderSelect(const Math::Cameras::FieldOfView &FOV) = 0;
    virtual ~ISelectionProvider() {}

    std::vector<xDWORD> *Select(const Math::Cameras::Camera &Camera, int X, int Y, int W = 1, int H = 1);
};

#endif
