#include <fstream>
#include "../../Utils/Filesystem.h"
#include "../../Utils/Utils.h"

#include "CameraSet.h"
#include "CameraFree.h"
#include "CameraHuman.h"

using namespace Math::Cameras;

void CameraSet :: Load(const char *fileName)
{
    Init();

    std::ifstream in;
    in.open(Filesystem::GetFullPath(fileName).c_str());
    if (in.is_open())
    {
        std::string dir = Filesystem::GetParentDir(fileName);
        char buffer[255];
        int  len;

        Camera        *camera = NULL;
        ObjectTracker *tracker = NULL;

        enum LoadMode
        {
            LoadMode_None,
            LoadMode_CameraHuman,
            LoadMode_CameraFree,
            LoadMode_Tracking,
            LoadMode_Viewport,
            LoadMode_FOV,
        } mode = LoadMode_None;

        while (in.good())
        {
            in.getline(buffer, 255);
            if (buffer[0] == 0 || buffer[0] == '#') continue;
            len = strlen(buffer);
            if (buffer[len - 1] == '\r') buffer[len - 1] = 0;

            if (*buffer == '[')
            {
                if (StartsWith(buffer, "[camera human]"))
                {
                    if (camera)
                        L_cameras.push_back(camera);
                    camera = new CameraHuman();
                    camera->Init();
                    camera->FOV.Projection = FieldOfView::PROJECT_ORTHOGONAL;
                    camera->FOV.FrontClip  = 0.1f;
                    camera->FOV.BackClip   = 1000.f;
                    mode = LoadMode_CameraHuman;
                    continue;
                }
                if (StartsWith(buffer, "[camera free]"))
                {
                    if (camera)
                        L_cameras.push_back(camera);
                    camera = new CameraFree();
                    camera->Init();
                    camera->FOV.Projection = FieldOfView::PROJECT_ORTHOGONAL;
                    camera->FOV.FrontClip  = 0.1f;
                    camera->FOV.BackClip   = 1000.f;
                    mode = LoadMode_CameraFree;
                    continue;
                }
                mode = LoadMode_None;
                continue;
            }
            if (camera)
            {
                if (StartsWith(buffer, "(track eye)"))
                {
                    tracker = &camera->EyeTracker;
                    mode = LoadMode_Tracking;
                    continue;
                }
                if (StartsWith(buffer, "(track center)"))
                {
                    tracker = &camera->CenterTracker;
                    mode = LoadMode_Tracking;
                    continue;
                }
                if (StartsWith(buffer, "(viewport)"))
                {
                    mode = LoadMode_Viewport;
                    continue;
                }
                if (StartsWith(buffer, "(fov)"))
                {
                    mode = LoadMode_FOV;
                    camera->FOV.Projection = FieldOfView::PROJECT_ORTHOGONAL;
                    continue;
                }

                if (StartsWith(buffer, "name"))
                {
                    camera->SN_name = ReadSubstring(buffer+4);
                    continue;
                }
                if (StartsWith(buffer, "eye"))
                {
                    sscanf(buffer+3, "%f\t%f\t%f", &camera->P_eye.x, &camera->P_eye.y, &camera->P_eye.z);
                    continue;
                }
                if (StartsWith(buffer, "center"))
                {
                    sscanf(buffer+6, "%f\t%f\t%f", &camera->P_center.x, &camera->P_center.y, &camera->P_center.z);
                    continue;
                }
                if (StartsWith(buffer, "up"))
                {
                    sscanf(buffer+2, "%f\t%f\t%f", &camera->NW_up.x, &camera->NW_up.y, &camera->NW_up.z);
                    continue;
                }
                if (StartsWith(buffer, "speedE"))
                {
                    sscanf(buffer+5, "%f", &camera->W_TrackingSpeedEye);
                    continue;
                }
                if (StartsWith(buffer, "speedC"))
                {
                    sscanf(buffer+5, "%f", &camera->W_TrackingSpeedCtr);
                    continue;
                }
            }
            if (mode == LoadMode_Tracking)
            {
                if (StartsWith(buffer, "mode"))
                {
                    const char *name = ReadSubstring(buffer+4);
                    if (StartsWith(name, "nothing"))    tracker->Mode = ObjectTracker::TRACK_NOTHING;
                    else
                    if (StartsWith(name, "object"))     tracker->Mode = ObjectTracker::TRACK_OBJECT;
                    else
                    if (StartsWith(name, "subobject"))  tracker->Mode = ObjectTracker::TRACK_SUBOBJECT;
                    else
                    if (StartsWith(name, "all_center")) tracker->Mode = ObjectTracker::TRACK_ALL_CENTER;
                    else
                    {
                        tracker->Mode = ObjectTracker::TRACK_CUSTOM_SCRIPT;
                        tracker->ScriptName = name;
                        if (StartsWith(name, "EyeSeeAll_CenterTop"))
                            tracker->Script = Camera::SCRIPT_EyeSeeAll_CenterTop;
                        else
                        if (StartsWith(name, "EyeSeeAll_Center"))
                            tracker->Script = Camera::SCRIPT_EyeSeeAll_Center;
                        else
                        if (StartsWith(name, "EyeSeeAll_Radius"))
                            tracker->Script = Camera::SCRIPT_EyeSeeAll_Radius;

                    }
                    continue;
                }
                if (StartsWith(buffer, "object"))
                {
                    int id;
                    sscanf(buffer+6, "%d", &id);
                    tracker->ID_object = id;
                    continue;
                }
                if (StartsWith(buffer, "subobj"))
                {
                    int id;
                    sscanf(buffer+6, "%d", &id);
                    tracker->ID_subobject = id;
                    continue;
                }
                if (StartsWith(buffer, "shift"))
                {
                    sscanf(buffer+5, "%f\t%f\t%f", &tracker->NW_destination_shift.x,
                        &tracker->NW_destination_shift.y, &tracker->NW_destination_shift.z);
                    continue;
                }
            }
            if (mode == LoadMode_Viewport)
            {
                if (StartsWith(buffer, "left"))
                {
                    int id;
                    sscanf(buffer+4, "%d", &id);
                    camera->FOV.ViewportLeftPercent = id * 0.01f;
                    continue;
                }
                if (StartsWith(buffer, "top"))
                {
                    int id;
                    sscanf(buffer+3, "%d", &id);
                    camera->FOV.ViewportTopPercent = id * 0.01f;
                    continue;
                }
                if (StartsWith(buffer, "width"))
                {
                    int id;
                    sscanf(buffer+5, "%d", &id);
                    camera->FOV.ViewportWidthPercent = id * 0.01f;
                    continue;
                }
                if (StartsWith(buffer, "height"))
                {
                    int id;
                    sscanf(buffer+6, "%d", &id);
                    camera->FOV.ViewportHeightPercent = id * 0.01f;
                    continue;
                }
            }
            if (mode == LoadMode_FOV)
            {
                if (StartsWith(buffer, "angle"))
                {
                    xFLOAT angle;
                    sscanf(buffer+5, "%f", &angle);
                    camera->FOV.PerspAngle = angle;
                    camera->FOV.Projection = FieldOfView::PROJECT_PERSPECTIVE;
                    camera->FOV.BackClip   = xFLOAT_HUGE_POSITIVE;
                    continue;
                }
                if (StartsWith(buffer, "front"))
                {
                    xFLOAT clip;
                    sscanf(buffer+5, "%f", &clip);
                    camera->FOV.FrontClip = clip;
                    continue;
                }
                if (StartsWith(buffer, "back"))
                {
                    xFLOAT clip;
                    sscanf(buffer+4, "%f", &clip);
                    camera->FOV.BackClip = clip;
                    continue;
                }
            }
        }
        if (camera)
            L_cameras.push_back(camera);

        in.close();
    }
}
