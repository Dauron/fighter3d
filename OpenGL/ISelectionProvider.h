#ifndef __incl_ISelectionProvider_h
#define __incl_ISelectionProvider_h

#include <vector>
#include "../Math/xMath.h"
#include "../Math/xFieldOfView.h"

class ISelectionProvider
{
  public:
    virtual unsigned int CountSelectable() = 0;
    virtual void RenderSelect(const xFieldOfView *FOV) = 0;
    virtual ~ISelectionProvider() {}

    std::vector<xDWORD> *Select(const xFieldOfView *FOV, int X, int Y, int W = 1, int H = 1);
};

#endif
