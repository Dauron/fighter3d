#include "Utils.h"

#ifdef USE_D3D

void LightSet_DX(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular, xBYTE light_id)
{
    // turn off ambient lighting
    //float light_off[4] = { 0.f, 0.f, 0.f, 0.f };
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_off);

    D3DLIGHT9 dxLight;    // create the light struct
    ZeroMemory(&dxLight, sizeof(dxLight));    // clear out the light struct for use

    if (t_Ambient)
    {
        dxLight.Ambient.r = light.diffuse.r;
        dxLight.Ambient.g = light.diffuse.g;
        dxLight.Ambient.b = light.diffuse.b;
        dxLight.Ambient.a = light.diffuse.a;
    }
    else
    {
        dxLight.Ambient.r = 0.f;
        dxLight.Ambient.g = 0.f;
        dxLight.Ambient.b = 0.f;
        dxLight.Ambient.a = 0.f;
    }
    if (t_Diffuse)
    {
        dxLight.Diffuse.r = light.diffuse.r;
        dxLight.Diffuse.g = light.diffuse.g;
        dxLight.Diffuse.b = light.diffuse.b;
        dxLight.Diffuse.a = light.diffuse.a;
    }
    else
    {
        dxLight.Diffuse.r = 0.f;
        dxLight.Diffuse.g = 0.f;
        dxLight.Diffuse.b = 0.f;
        dxLight.Diffuse.a = 0.f;
    }
    if (t_Specular)
    {
        dxLight.Specular.r = light.diffuse.r;
        dxLight.Specular.g = light.diffuse.g;
        dxLight.Specular.b = light.diffuse.b;
        dxLight.Specular.a = light.diffuse.a;
    }
    else
    {
        dxLight.Specular.r = 0.f;
        dxLight.Specular.g = 0.f;
        dxLight.Specular.b = 0.f;
        dxLight.Specular.a = 0.f;
    }

    if (light.type != xLight_INFINITE)
    {
        dxLight.Position = *(D3DVECTOR*)& light.position;
        dxLight.Range = light.radius;

        // rozpraszanie siê œwiat³a
        dxLight.Attenuation0 = light.attenuationConst;
        dxLight.Attenuation1 = light.attenuationLinear;
        dxLight.Attenuation2 = light.attenuationSquare;

        if (light.type == xLight_SPOT)
        {
            dxLight.Direction = *(D3DVECTOR*)& light.spotDirection;
            dxLight.Theta     = 0.f;                // inner angle
            dxLight.Phi       =  light.spotCutOff;  // outer angle
            dxLight.Falloff   = light.spotAttenuation;
            dxLight.Type = D3DLIGHT_SPOT;
        }
        else
            dxLight.Type = D3DLIGHT_POINT;
    }
    else
    {
        dxLight.Type = D3DLIGHT_DIRECTIONAL;
        dxLight.Direction = *(D3DVECTOR*)& -light.position;
    }

    //Shader::SetLightType(light.type, t_Ambient, t_Diffuse, t_Specular);

    Graphics::D3D::d3ddev->SetLight(light_id, &dxLight);
    Graphics::D3D::d3ddev->LightEnable(light_id, TRUE);
}

#endif
