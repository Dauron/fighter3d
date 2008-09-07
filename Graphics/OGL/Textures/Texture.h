#ifndef __incl_Texture_h
#define __incl_Texture_h

#include <string>
#include "../../../Utils/Resource.h"

struct Texture : public Resource
{
public:
    std::string  Name;         // for reconstruction
    bool         FL_MipMap;    // generate mip maps?
    unsigned int Width;        // width
    unsigned int Height;       // height
    unsigned int ID_GLTexture; // GL Texture identifier

    Texture() { Clear(); }

    virtual void Clear() {
        Resource::Clear();
        Name.clear();
        ID_GLTexture = 0;
    }

    virtual bool Create();
    virtual bool Create( const std::string& name, bool fl_mipmap )
    {
        Name      = name;
        FL_MipMap = fl_mipmap;
        return Create();
    }

    virtual void Dispose();
    virtual void Invalidate()
    { ID_GLTexture = 0; }
    virtual bool IsDisposed()
    { return ID_GLTexture == 0; }

    virtual const std::string &Identifier() { return Name; }
};

#endif
