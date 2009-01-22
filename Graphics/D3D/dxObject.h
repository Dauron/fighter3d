#ifndef __incl_Graphics_D3D_dxObject_h
#define __incl_Graphics_D3D_dxObject_h

#include "d3d.h"

namespace Graphics { namespace D3D {

#define FVFxVertex     D3DFVF_XYZ
#define FVFxVertexTex (D3DFVF_XYZ | D3DFVF_TEX1)

    struct dxObject {

    private:
        LPDIRECT3DVERTEXBUFFER9 ID_buffer;
        xDWORD                  stride;
        xDWORD                  count;
        xDWORD                  size;
        xDWORD                  fvf;

    public:
        bool IsValid() { return ID_buffer; }
        void Clear()   { ID_buffer = NULL; }

        bool Create(xDWORD stride, xDWORD count, DWORD fvf = FVFxVertex)
        {
            this->size   = stride * count;
            this->count  = count;
            this->stride = stride;
            this->fvf    = fvf;

            return Graphics::D3D::d3ddev->CreateVertexBuffer(
                          size,
                          0,    // static (non D3DUSAGE_DYNAMIC)
                          fvf,  // vertex format
                          D3DPOOL_MANAGED,
                          &ID_buffer,
                          NULL) == D3D_OK;
        }

        void Destroy()
        {
            if (ID_buffer) { ID_buffer->Release(); ID_buffer = NULL; }
        }

        void Load(void *data)
        {
            void* pVoid;    // the void pointer

            ID_buffer->Lock(0, 0, &pVoid, 0); // lock the vertex buffer
            memcpy(pVoid, data, size);        // copy the vertices to the locked buffer
            ID_buffer->Unlock();              // unlock the vertex buffer
        }

        void Use(xDWORD offset = 0)
        {
            Graphics::D3D::d3ddev->SetStreamSource(0, ID_buffer, offset, stride);
            Graphics::D3D::d3ddev->SetFVF(fvf);
        }
    };

}} // namespace Graphics::D3D

#endif
