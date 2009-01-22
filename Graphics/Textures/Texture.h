#ifndef __incl_Graphics_Texture_h
#define __incl_Graphics_Texture_h

#include <string>
#include <cstring> // memcpy
#include "../../Utils/Resource.h"
#include "ImageFile.h"

namespace Graphics {

    struct Texture : public Resource
    {
        Image              image;

    public:
        std::string  Name;         // for reconstruction
        bool         FL_MipMap;    // generate mip maps?
        unsigned int Width;        // width
        unsigned int Height;       // height

        Texture() { Clear(); }

        virtual void Clear() {
            Resource::Clear();
            Name.clear();
            image.data = 0;
        }

        bool Create( const std::string& name, bool fl_mipmap )
        {
            Name      = name;
            FL_MipMap = fl_mipmap;
            return Create();
        }
        bool Create( const std::string& name, Image image )
        {
            Name      = name;
            FL_MipMap = image.mipmap;

            this->image = image;
            unsigned int Bpp = image.bpp >> 3;
            unsigned int imageSize = image.sizeX * image.sizeY * Bpp;
            this->image.data = new unsigned char[imageSize];
            memcpy(this->image.data, image.data, imageSize);
            return Create();
        }

        virtual bool Create()  { return Resource::Create(); }
        virtual void Bind() = 0;

        virtual const std::string &Identifier() { return Name; }
    };

} // namespace Graphics

#endif
