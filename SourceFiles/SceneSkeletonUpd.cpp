#include "SceneSkeleton.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../Utils/Filesystem.h"
#include "../Physics/Verlet/VConstraintLengthEql.h"
#include "../Physics/Verlet/VConstraintLengthMin.h"
#include "../Physics/Verlet/VConstraintLengthMax.h"
#include "../Physics/Verlet/VConstraintAngular.h"

#include "../Math/Figures/xSphere.h"
#include "../Math/Figures/xCapsule.h"
#include "../Math/Figures/xBoxO.h"
#include "../Models/lib3dx/xUtils.h"

using namespace Scenes;
using namespace Math::Figures;

#define MULT_MOVE   5.0f
#define MULT_RUN    2.0f
#define MULT_ROT    80.0f
#define MULT_STEP   60.0f

void SceneSkeleton::SwitchDisplayedModel()
{
    State.DisplayPhysical = !State.DisplayPhysical;
    if (Model.ModelPh)
    {
        RigidObj::CopySpine(Model.ModelGr->xModelP->Spine, Model.ModelPh->xModelP->Spine);
        Model.SwapModels();
        Model.VerticesChanged(false, true);
    }
    Buttons[emMain][4].Down = State.DisplayPhysical;
}

void SceneSkeleton::UpdateBBox()
{
    xModel         &model         = *Model.ModelGr->xModelP;
    xModelInstance &modelInstance = Model.ModelGr->instance;

    xBoxA box;
    box.P_min.init(xFLOAT_HUGE_POSITIVE,xFLOAT_HUGE_POSITIVE,xFLOAT_HUGE_POSITIVE);
    box.P_max.init(xFLOAT_HUGE_NEGATIVE,xFLOAT_HUGE_NEGATIVE,xFLOAT_HUGE_NEGATIVE);

    for (int i = 0; i < modelInstance.I_elements; ++i)
    {
        xElementInstance &ei = modelInstance.L_elements[i];
        ei.Transform(model.L_kids->ById(i)->MX_MeshToLocal);

        xPoint3 P_min, P_max;
        ei.bBox_T->ComputeSpan(xVector3::Create(1,0,0), P_min.x, P_max.x);
        ei.bBox_T->ComputeSpan(xVector3::Create(0,1,0), P_min.y, P_max.y);
        ei.bBox_T->ComputeSpan(xVector3::Create(0,0,1), P_min.z, P_max.z);

        if (P_min == P_max) continue;
        if (P_min.x < box.P_min.x) box.P_min.x = P_min.x;
        if (P_min.y < box.P_min.y) box.P_min.y = P_min.y;
        if (P_min.z < box.P_min.z) box.P_min.z = P_min.z;
        if (P_max.x > box.P_max.x) box.P_max.x = P_max.x;
        if (P_max.y > box.P_max.y) box.P_max.y = P_max.y;
        if (P_max.z > box.P_max.z) box.P_max.z = P_max.z;
    }
    Animation.Skeleton.Bounds = box;
}

void SceneSkeleton::UpdateCustomBVH()
{
    xModel         &model         = *Model.ModelGr->xModelP;
    xModelInstance &modelInstance = Model.ModelGr->instance;
    xBVHierarchy   *BVHierarchy = model.BVHierarchy;

    if (BVHierarchy)
    {
        BVHierarchy->invalidateTransformation();

        if (model.Spine.I_bones && modelInstance.MX_bones)
            for (int i = 0; i < BVHierarchy->I_items; ++i)
                BVHierarchy->L_items[i].MX_RawToLocal_Set(xMatrix::Transpose( modelInstance.MX_bones[BVHierarchy->L_items[i].ID_Bone] ));
        else
            for (int i = 0; i < BVHierarchy->I_items; ++i)
                BVHierarchy->L_items[i].MX_RawToLocal_Set( xMatrix::Identity() );

        Math::Figures::xBoxA box = BVHierarchy->childBounds();
        Math::Figures::xSphere &sphere   = *(Math::Figures::xSphere*) BVHierarchy->Figure;
        Math::Figures::xSphere &sphere_T = *(Math::Figures::xSphere*) BVHierarchy->GetTransformed(xMatrix::Identity());
        sphere.S_radius = sphere_T.S_radius = (box.P_max - box.P_min).length() * 0.5f;
        sphere_T.P_center = (box.P_max + box.P_min) * 0.5f;
        sphere.P_center = sphere_T.P_center;
    }
}

/************************** INPUT **************************************/
bool SceneSkeleton::Update(float deltaTime)
{
    InputMgr &im = g_InputMgr;

    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        if (im.GetInputStateAndClear(begin->Action))
        { UpdateButton(*begin); return true; }

    if (im.GetInputStateAndClear(IC_FullScreen)) {
        if (g_Application.MainWindow_Get().IsFullScreen())
            g_Application.MainWindow_Get().FullScreen_Set(Config::WindowX, Config::WindowY, false);
        else
            g_Application.MainWindow_Get().FullScreen_Set(Config::FullScreenX, Config::FullScreenY, true);
        return true;
    }

    if (im.GetInputStateAndClear(IC_Reject))
    {
        if (EditMode == emSelectElement || EditMode == emCreateBone || EditMode == emSelectAnimation)
            EditMode = emMain;
        else
        if (EditMode == emCreateConstraint_Type)
            EditMode = emCreateBone;
        else
        if (EditMode == emCreateConstraint_Node)
            EditMode = emCreateConstraint_Type;
        else
        if (EditMode == emCreateConstraint_Params)
        {
            Constraint.boneB = xBYTE_MAX;
            EditMode = emCreateConstraint_Node;
        }
        else
        if (EditMode == emEditBVH)
            EditMode = emMain;
        else
        if (EditMode == emCreateBVH || EditMode == emSelectBVHBone || EditMode == emEditVolume)
        {
            EditMode = emEditBVH;
            Selection.BVHNode   = NULL;
            Selection.BVHNodeID = xBYTE_MAX;
        }
        else
        if (EditMode == emSelectVertex)
        {
            EditMode = emSelectElement;
            Selection.ElementId = xWORD_MAX;
        }
        else
        if (EditMode == emSelectBone)
            EditMode = emSelectVertex;
        else
        if (EditMode == emInputWght) {
            EditMode = emSelectBone;
            Selection.Bone = NULL;
        }
        else
        if (EditMode == emEditAnimation) {
            EditMode = emSelectAnimation;
            Animation.Instance->Unload();
            delete Animation.Instance;
            Animation.Instance = NULL;
        }
        else
        if (EditMode == emAnimateBones ||
            EditMode == emFrameParams ||
            EditMode == emLoadAnimation||
            EditMode == emSaveAnimation) {
            Directories.clear();
            EditMode = emEditAnimation;
        }
        else {
            Scene &tmp = *PrevScene;
            PrevScene = NULL;
            g_Application.Scene_Set(tmp);
        }
        return true;
    }

    UpdateMouse(deltaTime);

    if (EditMode == emSelectVertex && im.GetInputStateAndClear(IC_Accept)) {
        if (Selection.Vertices.size()) {
            EditMode = emSelectBone;
            Selection.Bone = NULL;
            memset(&Skin, 0, sizeof(Skin));
            return true;
        }
    }
    if (EditMode != emSaveModel && EditMode != emSaveAnimation && EditMode != emFrameParams &&
        EditMode != emInputWght && EditMode != emCreateConstraint_Params)
    {
        if (im.GetInputStateAndClear(IC_ShowBonesAlways))
            State.ShowBonesAlways = !State.ShowBonesAlways;
        if (EditMode != emSelectVertex && EditMode != emSelectBone &&
            im.GetInputStateAndClear(IC_ViewPhysicalModel))
        {
            SwitchDisplayedModel();
            return true;
        }
    }
    if (EditMode == emCreateConstraint_Params)
    {
        GetConstraintParams();
        return true;
    }
    if (EditMode == emInputWght)
    {
        GetBoneIdAndWeight();
        return true;
    }
    if (EditMode == emFrameParams)
    {
        GetFrameParams();
        return true;
    }
    if (EditMode == emSaveAnimation || EditMode == emSaveModel)
    {
        GetCommand();
        return true;
    }
    if (EditMode == emEditAnimation && State.PlayAnimation)
    {
        if (im.GetInputState(IC_BE_Modifier))
            Animation.Instance->T_progress += (xWORD)(deltaTime*100);
        else
            Animation.Instance->T_progress += (xWORD)(deltaTime*1000);
        Animation.Instance->UpdatePosition();
        if (! Animation.Instance->CurrentFrame)
            Animation.Instance->CurrentFrame = Animation.Instance->L_frames;
        Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
        Model.CalculateSkeleton();

        UpdateCustomBVH();
    }

    if (EditMode == emEditAnimation)
    {
        if (im.GetInputState(IC_BE_Modifier))
        {
            bool update = false;
            if (! Animation.Instance->CurrentFrame)
                Animation.Instance->CurrentFrame = Animation.Instance->L_frames;
            if (im.GetInputState(IC_MoveLeft))
            { Animation.Instance->T_progress -= 1; update = true; }
            if (im.GetInputState(IC_MoveRight))
            { Animation.Instance->T_progress += 1; update = true; }
            if (im.GetInputStateAndClear(IC_MoveUp))
            {
                if (Animation.Instance->CurrentFrame->Next)
                {
                    Animation.Instance->CurrentFrame = Animation.Instance->CurrentFrame->Next;
                    Animation.Instance->T_progress = 0;
                }
                else
                    Animation.Instance->T_progress = Animation.Instance->CurrentFrame->T_freeze + Animation.Instance->CurrentFrame->T_duration;
                update = true;
            }
            if (im.GetInputStateAndClear(IC_MoveDown))
            {
                if (Animation.Instance->T_progress)
                    Animation.Instance->T_progress = 0;
                else
                if (Animation.Instance->CurrentFrame->Prev)
                    Animation.Instance->CurrentFrame = Animation.Instance->CurrentFrame->Prev;
                update = true;
            }

            if (update)
            {
                Animation.Instance->UpdatePosition();
                if (! Animation.Instance->CurrentFrame)
                    Animation.Instance->CurrentFrame = Animation.Instance->L_frames;
                Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
                Model.CalculateSkeleton();

                UpdateCustomBVH();
                return true;
            }
        }
    }

    UpdateDisplay(deltaTime);
    return true;
}

bool SceneSkeleton::UpdateButton(GLButton &button)
{
    if (button.RadioBox)
    {
        std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
        std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
        for (; begin != end; ++begin)
            if (begin->RadioBox)
                begin->Down = button.Action == begin->Action;
        State.CurrentAction = button.Action;
    }

    if (EditMode == emMain)
    {
        if (button.Action == IC_BE_ModeSkeletize) {
            EditMode = emCreateBone;
            UpdateButton(Buttons[emCreateBone][0]);
            Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones;
            Model.ModelGr->xModelP->Spine.ResetQ();
            Model.CalculateSkeleton();

            xBVHierarchy *root = Model.ModelGr->xModelP->BVHierarchy;
            if (root)
                for (int i = 0; i < root->I_items; ++i)
                    root->L_items[i].MX_RawToLocal_Set(xMatrix::Identity());
        }
        else
        if (button.Action == IC_BE_ModeBVH)
        {
            EditMode = emEditBVH;
            UpdateButton(Buttons[emEditBVH][1]);
            Model.ModelGr->xModelP->Spine.ResetQ();
            Model.CalculateSkeleton();

            xBVHierarchy *root = Model.ModelGr->xModelP->BVHierarchy;
            if (root)
                for (int i = 0; i < root->I_items; ++i)
                    root->L_items[i].MX_RawToLocal_Set(xMatrix::Identity());
        }
        else // must be skeletized to perform skinning
        if (button.Action == IC_BE_ModeSkin /*&& Model.ModelGr->xModelP->Spine.L_bones*/)
            EditMode = emSelectElement;
        else
        if (button.Action == IC_BE_ModeAnimate)
            EditMode = emSelectAnimation;
        else
        if (button.Action == IC_BE_Select)
            SwitchDisplayedModel();
        else
        if (button.Action == IC_BE_Save)
        {
            if (State.DisplayPhysical)
                SwitchDisplayedModel();
            CurrentDirectory = Filesystem::GetParentDir(Model.ModelGr->xModelP->FileName);
            InputState.String = Filesystem::GetFileName(Model.ModelGr->xModelP->FileName);
            SaveModel.FL_save_physical = false;
            FillDirectoryBtns(true, "*.3dx");
            EditMode = emSaveModel;
            g_InputMgr.Buffer.clear();
        }

        return true;
    }
    if (EditMode == emCreateBone)
    {
        if (button.Action == IC_BE_Delete && Selection.Bone)
        {
            if (Model.ModelPh)
                RigidObj::CopySpine(Model.ModelGr->xModelP->Spine, Model.ModelPh->xModelP->Spine);
            xBYTE boneId = Selection.Bone->ID;
            Selection.Bone = NULL;
            Model.ModelGr->xModelP->BoneDelete(boneId);
            if (Model.ModelPh)
                Model.ModelPh->xModelP->BoneDelete(boneId);
            Model.VerticesChanged(false, true);
        }
        if (button.Action == IC_BE_CreateConstr)
            EditMode = emCreateConstraint_Type;
        return true;
    }
    if (EditMode == emCreateConstraint_Type)
    {
        if (button.Action == IC_BE_CreateConstrMax ||
            button.Action == IC_BE_CreateConstrMin ||
            button.Action == IC_BE_CreateConstrEql ||
            button.Action == IC_BE_CreateConstrAng ||
            button.Action == IC_BE_CreateConstrWeight)
        {
            Constraint.boneA = Constraint.boneB = xBYTE_MAX;
            Constraint.type = button.Action;
            EditMode = emCreateConstraint_Node;
        }
        return true;
    }
    if (EditMode == emEditBVH)
    {
        if (button.Action == IC_BE_Create)
        {
            Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones;
            if (Model.ModelGr->xModelP->Spine.L_bones)
                EditMode = emSelectBVHBone;
            else
                EditMode = emCreateBVH;
        }
        return true;
    }
    if (EditMode == emCreateBVH)
    {
        xModel &model = *Model.ModelGr->xModelP;
        if (!model.BVHierarchy)
        {
            model.BVHierarchy = new xBVHierarchy();
            model.BVHierarchy->init(*(new xSphere()));
            xSphere &sphere = *(xSphere*) model.BVHierarchy->Figure;

            if (model.Spine.L_bones)
                sphere.P_center = model.Spine.L_bones[0].P_begin;
            else
                sphere.P_center.zero();
            sphere.S_radius = 0.f;
        }
        Selection.BVHNode = model.BVHierarchy;

        if (button.Action == IC_BE_CreateSphere)
        {
            xSphere *sphere = new xSphere();
            Selection.BVHNode->add(*sphere);
            Selection.BVHNode = Selection.BVHNode->L_items + (Selection.BVHNode->I_items-1);
            if (model.Spine.L_bones)
            {
                Selection.BVHNode->ID_Bone = Selection.Bone->ID;
                sphere->P_center = 0.5f * (model.Spine.L_bones[Selection.Bone->ID].P_begin + model.Spine.L_bones[Selection.Bone->ID].P_end);
                sphere->S_radius = 0.5f * (model.Spine.L_bones[Selection.Bone->ID].P_begin - model.Spine.L_bones[Selection.Bone->ID].P_end).length();
                if (sphere->S_radius < 0.01f) sphere->S_radius = 0.1f;

            }
            else
            {
                Selection.BVHNode->ID_Bone = 0;
                sphere->P_center.zero();
                sphere->S_radius = 0.1f;
            }
        }
        if (button.Action == IC_BE_CreateCapsule)
        {
            xCapsule *capsule = new xCapsule();
            Selection.BVHNode->add(*capsule);
            Selection.BVHNode = Selection.BVHNode->L_items + (Selection.BVHNode->I_items-1);
            if (model.Spine.L_bones)
            {
                Selection.BVHNode->ID_Bone = Selection.Bone->ID;
                capsule->S_radius = 0.05f;
                capsule->P_center = 0.5f * (model.Spine.L_bones[Selection.Bone->ID].P_begin + model.Spine.L_bones[Selection.Bone->ID].P_end);
                capsule->N_top    = 0.5f * (model.Spine.L_bones[Selection.Bone->ID].P_begin - model.Spine.L_bones[Selection.Bone->ID].P_end);
                capsule->S_top    = capsule->N_top.length();
                capsule->N_top.normalize();
                if (capsule->S_top < 0.01f) { capsule->S_top = 0.1f; capsule->N_top.init(0,0,1); }
            }
            else
            {
                Selection.BVHNode->ID_Bone = 0;
                capsule->P_center.zero();
                capsule->S_radius = 0.05f;
                capsule->S_top = 0.1f;
                capsule->N_top.init(0,0,1);
            }
        }
        if (button.Action == IC_BE_CreateBox)
        {
            xBoxO *boxO = new xBoxO();
            Selection.BVHNode->add(*boxO);
            Selection.BVHNode = Selection.BVHNode->L_items + (Selection.BVHNode->I_items-1);
            if (model.Spine.L_bones)
            {
                Selection.BVHNode->ID_Bone = Selection.Bone->ID;
                boxO->S_side   = 0.05f;
                boxO->S_front  = 0.05f;
                boxO->N_front.init(0,1,0);
                boxO->P_center = 0.5f * (model.Spine.L_bones[Selection.Bone->ID].P_begin + model.Spine.L_bones[Selection.Bone->ID].P_end);
                boxO->N_top    = 0.5f * (model.Spine.L_bones[Selection.Bone->ID].P_begin - model.Spine.L_bones[Selection.Bone->ID].P_end);
                boxO->S_top    = boxO->N_top.length();
                boxO->N_top.normalize();
                if (boxO->S_top < 0.01f) { boxO->S_top = 0.1f; boxO->N_top.init(0,0,1); }
                boxO->N_side = xVector3::CrossProduct(boxO->N_top, boxO->N_front);
                boxO->N_front = xVector3::CrossProduct(boxO->N_top, boxO->N_side);
            }
            else
            {
                Selection.BVHNode->ID_Bone = 0;
                boxO->P_center.zero();
                boxO->S_side  = 0.05f;
                boxO->S_front = 0.05f;
                boxO->S_top   = 0.1f;
                boxO->N_top.init(0,0,1);
                boxO->N_front.init(0,1,0);
                boxO->N_side.init(1,0,0);
            }
        }
        xBYTE cid = 0;
        Selection.BVHNodeID = GetBVH_ID(*model.BVHierarchy, Selection.BVHNode, cid);
        EditMode = emEditVolume;
    }
    if (EditMode == emSelectAnimation)
    {
        if (button.Action == IC_BE_Create)
        {
            Animation.Instance  = new xAnimation();
            Animation.Instance->Reset(Model.ModelGr->xModelP->Spine.I_bones);
            Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
            Model.CalculateSkeleton();
            UpdateCustomBVH();
            Buttons[emEditAnimation][5].Down = false;
            EditMode = emEditAnimation;
            AnimationName.clear();
        }
        else
        if (button.Action == IC_BE_Select)
        {
            FillDirectoryBtns(true, "*.ska");
            EditMode = emLoadAnimation;
        }
        return true;
    }
    if (EditMode == emEditAnimation)
    {
        if (button.Action == IC_BE_Play)
            button.Down = State.PlayAnimation = !button.Down;
        if (button.Action == IC_BE_Create)
        {
            Animation.Instance->InsertKeyFrame();
            EditMode = emAnimateBones;
            Buttons[emAnimateBones][3].Down = !(State.HideBonesOnAnim = false);
            UpdateButton(Buttons[emAnimateBones][0]);
        }
        if (button.Action == IC_BE_Edit)
        {
            EditMode = emAnimateBones;
            Buttons[emAnimateBones][3].Down = !(State.HideBonesOnAnim = false);
            UpdateButton(Buttons[emAnimateBones][0]);
            Animation.Instance->T_progress = 0;
            Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
            Model.CalculateSkeleton();
            UpdateBBox();
        }
        if (button.Action == IC_BE_Move)
        {
            Animation.Instance->T_progress = 0;
            Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
            Model.CalculateSkeleton();
            EditMode = emFrameParams;
            g_InputMgr.Buffer.clear();
            InputState.String.clear();
            Animation.KeyFrame.step = 1;
            Animation.KeyFrame.freeze   = Animation.Instance->CurrentFrame->T_freeze;
            Animation.KeyFrame.duration = Animation.Instance->CurrentFrame->T_duration;
        }
        if (button.Action == IC_BE_Delete)
        {
            Animation.Instance->DeleteKeyFrame();
            if (! Animation.Instance->I_frames)
                Animation.Instance->InsertKeyFrame();
            Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
            Model.CalculateSkeleton();
        }
        else
        if (button.Action == IC_BE_Loop)
        {
            Buttons[emEditAnimation][5].Down = !Buttons[emEditAnimation][5].Down;
            if (Animation.Instance->L_frames->Prev)
            {
                Animation.Instance->L_frames->Prev->Next = NULL;
                Animation.Instance->L_frames->Prev = NULL;
            }
            else
            {
                xKeyFrame *frameP = Animation.Instance->L_frames;
                while (frameP->Next)
                    frameP = frameP->Next;
                Animation.Instance->L_frames->Prev = frameP;
                frameP->Next = Animation.Instance->L_frames;
            }
        }
        else
        if (button.Action == IC_BE_Save)
        {
            FillDirectoryBtns(true, "*.ska");
            EditMode = emSaveAnimation;
            InputState.String = AnimationName;
            g_InputMgr.Buffer.clear();
        }
        return true;
    }
    if (EditMode == emAnimateBones)
    {
        if (button.Action == IC_BE_ModeSkeletize)
            button.Down = !(State.HideBonesOnAnim = !State.HideBonesOnAnim);
        else
        if (button.Action == IC_BE_Delete && Selection.Bone)
        {
            Selection.Bone->QT_rotation.zeroQ();
            Model.CalculateSkeleton();
        }
        else
        if (button.Action == IC_BE_Save)
        {
            Animation.Instance->CurrentFrame->LoadFromSkeleton(Model.ModelGr->xModelP->Spine);
            UpdateCustomBVH();
            EditMode = emEditAnimation;
        }
        else
        if (button.Action == IC_Reject)
        {
            Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
            Model.CalculateSkeleton();
            EditMode = emEditAnimation;
        }
        return true;
    }
    if (EditMode == emSaveAnimation)
    {
        if (button.Action == IC_BE_Move)
        {
            if (!strcmp(button.Text, ".."))
                CurrentDirectory = Filesystem::GetParentDir(CurrentDirectory);
            else
            {
                CurrentDirectory += "/";
                CurrentDirectory += button.Text;
            }
            FillDirectoryBtns(true, "*.ska");
            return true;
        }
        if (button.Action == IC_BE_Select)
        {
            InputState.String = button.Text;
        }
        if (button.Action == IC_Accept)
        {
            if (InputState.String.size())
                AnimationName = InputState.String;
            if (AnimationName.size())
            {
                AnimationName = Filesystem::ChangeFileExt(AnimationName, "ska");
                std::string filePath = CurrentDirectory + "/";
                filePath += AnimationName;
                Animation.Instance->Save(filePath.c_str());
                EditMode = emEditAnimation;
            }
        }
        if (button.Action == IC_Reject)
        {
            Directories.clear();
            EditMode = emEditAnimation;
        }
        return true;
    }
    if (EditMode == emLoadAnimation)
    {
        if (button.Action == IC_BE_Move)
        {
            if (!strcmp(button.Text, ".."))
                CurrentDirectory = Filesystem::GetParentDir(CurrentDirectory);
            else
            {
                CurrentDirectory += "/";
                CurrentDirectory += button.Text;
            }
            FillDirectoryBtns(true, "*.ska");
            return true;
        }
        if (button.Action == IC_BE_Select)
        {
            AnimationName = button.Text;
            std::string filePath = CurrentDirectory + "/";
            filePath += AnimationName;
            Animation.Instance = new xAnimation();
            Animation.Instance->Load(filePath.c_str());
            Animation.Instance->SaveToSkeleton(Model.ModelGr->xModelP->Spine);
            Model.CalculateSkeleton();
            UpdateCustomBVH();
            EditMode = emEditAnimation;
            Buttons[emEditAnimation][5].Down = Animation.Instance->L_frames && Animation.Instance->L_frames->Prev;
        }
        if (button.Action == IC_Reject)
        {
            Directories.clear();
            EditMode = emSelectAnimation;
        }
        return true;
    }
    if (EditMode == emSaveModel)
    {
        if (button.Action == IC_BE_Move)
        {
            if (!strcmp(button.Text, ".."))
                CurrentDirectory = Filesystem::GetParentDir(CurrentDirectory);
            else
            {
                CurrentDirectory += "/";
                CurrentDirectory += button.Text;
            }
            FillDirectoryBtns(true, "*.3dx");
            return true;
        }
        if (button.Action == IC_BE_Select)
        {
            InputState.String = button.Text;
        }
        if (button.Action == IC_Accept)
        {
            if (!InputState.String.size())
                InputState.String = Model.ModelGr->xModelP->FileName;

            if (InputState.String.size())
            {
                if (Filesystem::GetFileExt(InputState.String) == "3ds")
                    InputState.String = Filesystem::ChangeFileExt(InputState.String, "3dx");

                InputState.String = CurrentDirectory + "/" + InputState.String;

                delete[] Model.ModelGr->xModelP->FileName;
                Model.ModelGr->xModelP->FileName = strdup (InputState.String.c_str());
                Model.ModelGr->xModelP->Save();

                if (Model.ModelPh && !SaveModel.FL_save_physical)
                {
                    SwitchDisplayedModel();
                    CurrentDirectory = Filesystem::GetParentDir(Model.ModelGr->xModelP->FileName);
                    InputState.String = Filesystem::GetFileName(Model.ModelGr->xModelP->FileName);
                    FillDirectoryBtns(true, "*.3dx");
                    g_InputMgr.Buffer.clear();
                    SaveModel.FL_save_physical = true;
                }
                else
                    EditMode = emMain;
            }
            return true;
        }
        if (button.Action == IC_Reject)
        {
            Directories.clear();
            EditMode = emMain;
            return true;
        }
    }
    return false;
}

/************************** MOUSE **************************************/
void SceneSkeleton::UpdateMouse(float deltaTime)
{
    InputMgr &im = g_InputMgr;

    if (InputState.LastX != im.mouseX || InputState.LastY != im.mouseY)
        MouseMove(im.mouseX, im.mouseY);

    if (!InputState.MouseLIsDown && im.GetInputState(IC_LClick)) {
        InputState.MouseLIsDown = true;
        MouseLDown(im.mouseX, im.mouseY);
        return;
    }

    if (InputState.MouseLIsDown && !im.GetInputState(IC_LClick)) {
        InputState.MouseLIsDown = false;
        MouseLUp(im.mouseX, im.mouseY);
    }

    if (!InputState.MouseRIsDown && im.GetInputState(IC_RClick)) {
        InputState.MouseRIsDown = true;
        MouseRDown(im.mouseX, im.mouseY);
        return;
    }

    if (InputState.MouseRIsDown && !im.GetInputState(IC_RClick)) {
        InputState.MouseRIsDown = false;
        MouseRUp(im.mouseX, im.mouseY);
    }
}

void SceneSkeleton::MouseLDown (int X, int Y)
{
    if (EditMode == emSelectVertex)
    {
        Selection.RectStartX = X;
        Selection.RectStartY = Y;
    }
    else if (EditMode == emAnimateBones && Selection.Bone)
        Animation.Skeleton.PreviousQuaternion = Selection.Bone->QT_rotation;
    else if (EditMode == emEditVolume)
        MouseLDown_BVH(X, Y);
}

void SceneSkeleton::MouseLUp   (int X, int Y)
{
    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        if (begin->Click((xFLOAT)X, (xFLOAT)Y) && UpdateButton(*begin))
            return;

    if (EditMode == emCreateBone)
    {
        if (State.CurrentAction == IC_BE_Create) // create bone
        {
            xVector3 hitPos = Selection.Bone
                ? Selection.Bone->P_end : xVector3::Create(0.0f, 0.0f, 0.0f);
            hitPos = Cameras.Current->FOV.Get3dPos(X, Height-Y, hitPos);

            if (!Model.ModelGr->xModelP->Spine.I_bones)              // if no spine
            {
                Model.ModelGr->xModelP->SkeletonAdd();               //   add skeleton to model
                if (Model.ModelPh)
                    Model.ModelPh->xModelP->SkeletonAdd();           //   add skeleton to model
                Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones; //   select root
                Selection.Bone->P_end = hitPos;                     //   set root position
            }
            else
            {
                if (!Selection.Bone) Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones;
                Selection.Bone = Model.ModelGr->xModelP->Spine.BoneAdd(Selection.Bone->ID, hitPos); // add bone to skeleton
                if (Model.ModelPh)
                    Model.ModelPh->xModelP->Spine.BoneAdd(Selection.Bone->ID, hitPos); // add bone to skeleton
            }
            Model.CalculateSkeleton();
        }
        else
        if (State.CurrentAction == IC_BE_DeleteConstr)
        {
            std::vector<xDWORD> *sel = SelectCommon(X, Y);
            if (sel)
            {
                if (sel->size())
                {
                    int selectedConstr = sel->back();
                    xSkeleton &spine = Model.ModelGr->xModelP->Spine;
                    if (spine.I_constraints == 1)
                    {
                        spine.I_constraints = 0;
                        delete *spine.C_constraints;
                        delete[] spine.C_constraints;
                        spine.C_constraints = NULL;
                    }
                    else
                    {
                        VConstraint** C_tmp = new VConstraint*[spine.I_constraints-1];
                        memcpy (C_tmp, spine.C_constraints, selectedConstr*sizeof(VConstraint*));
                        memcpy (C_tmp+selectedConstr, spine.C_constraints+selectedConstr+1, (spine.I_constraints-selectedConstr-1)*sizeof(VConstraint*));
                        delete spine.C_constraints[selectedConstr];
                        delete[] spine.C_constraints;
                        spine.C_constraints = C_tmp;
                        --spine.I_constraints;
                    }
                    if (Model.ModelPh)
                        RigidObj::CopySpine(Model.ModelGr->xModelP->Spine, Model.ModelPh->xModelP->Spine);
                }
                delete sel;
            }
        }
        else
        if (State.CurrentAction != IC_BE_Move) // select bone
        {
            Selection.Bone = SelectBone(X,Y);
            if (!Selection.Bone) Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones;
        }
    }
    else
    if (EditMode == emCreateConstraint_Node)
    {
        xBone *bone = SelectBone(X,Y);
        xBone *root = Model.ModelGr->xModelP->Spine.L_bones;
        if (!bone) bone = root;

        if (Constraint.type == IC_BE_CreateConstrAng)
        {
            if (bone->ID == 0 || (bone->ID_parent == 0 && root->I_kids == 1 )) return;
            Constraint.boneA = bone->ID;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrWeight)
            Constraint.boneA = bone->ID;
        else
        if (Constraint.boneA == xBYTE_MAX)
            Constraint.boneA = bone->ID;
        else
        if (Constraint.boneA != bone->ID)
            Constraint.boneB = bone->ID;

        Constraint.step = 0;

        if (Constraint.type == IC_BE_CreateConstrAng)
        {
            EditMode = emCreateConstraint_Params;
            Constraint.minX = Constraint.minY = -45;
            Constraint.maxX = Constraint.maxY = +45;
            InputState.String.clear();
            g_InputMgr.Buffer.clear();
        }
        else if (Constraint.type == IC_BE_CreateConstrWeight)
        {
            EditMode = emCreateConstraint_Params;
            Constraint.M_weight = bone->M_weight;
            InputState.String.clear();
            g_InputMgr.Buffer.clear();
        }
        else if (Constraint.boneB != xBYTE_MAX)
        {
            xBone *bone1 = Model.ModelGr->xModelP->Spine.L_bones + Constraint.boneA;
            Constraint.S_length = bone1->S_length;
            EditMode = emCreateConstraint_Params;
            InputState.String.clear();
            g_InputMgr.Buffer.clear();
        }
    }
    else
    if (EditMode == emEditBVH)
    {
        xBVHierarchy *bvh = SelectBVH(X,Y);
        if (bvh)
        {
            xModel &model = *Model.ModelGr->xModelP;
            if (State.CurrentAction == IC_BE_Edit)
            {
                Selection.BVHNode = bvh;
                xBYTE cid = 0;
                Selection.BVHNodeID = GetBVH_ID(*model.BVHierarchy, bvh, cid);
                EditMode = emEditVolume;
            }
            else
            if (State.CurrentAction == IC_BE_Clone)
            {
                Selection.BVHNode = model.BVHierarchy->add(*bvh->Figure->Transform(xMatrix::Identity()));
                xBYTE cid = 0;
                Selection.BVHNodeID = GetBVH_ID(*model.BVHierarchy, Selection.BVHNode, cid);
                EditMode = emEditVolume;
            }
            else
            if (State.CurrentAction == IC_BE_Delete)
            {
                model.BVHierarchy->remove(bvh);
                Selection.BVHNode   = NULL;
                Selection.BVHNodeID = xBYTE_MAX;
            }
        }
    }
    else
    if (EditMode == emSelectElement) // select element
    {
        Selection.ElementId = (int) SelectElement(X,Y);
        if (Selection.ElementId != xWORD_MAX) { // MAXUINT
            Selection.Element = Model.ModelGr->xModelP->L_kids->ById(Selection.ElementId);
            EditMode = emSelectVertex;
            Selection.Vertices.clear();
        }
        else
            Selection.Element = NULL;
    }
    else
    if (EditMode == emSelectVertex) // select vertices inside the selection box
    {
        // normalize the selection box
        int selEndX = max(Selection.RectStartX, X); Selection.RectStartX = min(Selection.RectStartX, X);
        int selEndY = max(Selection.RectStartY, Y); Selection.RectStartY = min(Selection.RectStartY, Y);
        std::vector<xDWORD> *sel = SelectCommon(Selection.RectStartX, Selection.RectStartY,
                                                selEndX-Selection.RectStartX, selEndY-Selection.RectStartY);
        // if there were vertices inside the selection box
        if (sel) {
            std::vector<xDWORD>::iterator iter, found;
            for (iter = sel->begin(); iter < sel->end(); ++iter)
            {
                found = std::find<std::vector<xDWORD>::iterator, xDWORD> // check if vertex is selected
                                        (Selection.Vertices.begin(), Selection.Vertices.end(), *iter);

                if (!g_InputMgr.GetInputState(IC_BE_Modifier)) // if selecting
                {
                    if (found == Selection.Vertices.end())       //   if not selected yet
                        Selection.Vertices.push_back(*iter);     //     add vertex to selection
                }
                else                                       // if unselecting
                    if (found != Selection.Vertices.end())       //   if selected
                        Selection.Vertices.erase(found);         //     remove vertex from selection
            }
            delete sel;
        }
    }
    else
    if (EditMode == emSelectBone) // select bone and switch to the Input Weight Mode
    {
        Selection.Bone = SelectBone(X,Y);
        if (!Selection.Bone) Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones;

        EditMode = emInputWght;
        g_InputMgr.Buffer.clear();
        InputState.String.clear();
    }
    else
    if (EditMode == emSelectBVHBone) // select bone and switch to the Create BVH Mode
    {
        Selection.Bone = SelectBone(X,Y);
        if (!Selection.Bone) Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones;

        EditMode = emCreateBVH;
    }
    else
    if (EditMode == emAnimateBones)
    {
        if (State.CurrentAction != IC_BE_Move) // select bone
        {
            Selection.Bone = SelectBone(X,Y);
            if (!Selection.Bone) Selection.Bone = Model.ModelGr->xModelP->Spine.L_bones;
            UpdateButton(Buttons[emAnimateBones][1]); // switch to move mode
        }
        else
            UpdateBBox();
    }
    else
    if (EditMode == emLoadAnimation || EditMode == emSaveAnimation || EditMode == emSaveModel)
    {
        begin = Directories.begin();
        end   = Directories.end();
        for (; begin != end; ++begin)
            if (begin->Click((xFLOAT)X, (xFLOAT)Y) && UpdateButton(*begin))
                return;
    }
}

void SceneSkeleton::MouseRDown (int X, int Y)
{
}
void SceneSkeleton::MouseRUp   (int X, int Y)
{
    if (EditMode == emSelectVertex)
        Selection.Vertices.clear();
}


void SceneSkeleton::MouseMove  (int X, int Y)
{
    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        begin->Hover((xFLOAT)X, (xFLOAT)Y);

    if (EditMode == emCreateBone && Selection.Bone && // edit-move bone (ending)
        InputState.MouseLIsDown && State.CurrentAction == IC_BE_Move)
    {
        Selection.Bone->P_end = Cameras.Current->FOV.Get3dPos(X, Height-Y, Selection.Bone->P_end);
        Selection.Bone->S_lengthSqr = (Selection.Bone->P_end-Selection.Bone->P_begin).lengthSqr();
        Selection.Bone->S_length    = sqrt(Selection.Bone->S_lengthSqr);
        xSkeleton &spine = Model.ModelGr->xModelP->Spine;
        xBYTE *join  = Selection.Bone->ID_kids;
        for (xBYTE i = Selection.Bone->I_kids; i; --i, ++join)
        {
            xBone &bone = spine.L_bones[*join];
            bone.P_begin = Selection.Bone->P_end;
            bone.S_lengthSqr = (bone.P_end-bone.P_begin).lengthSqr();
            bone.S_length    = sqrt(bone.S_lengthSqr);
        }
        Model.CalculateSkeleton();                           // refresh model in GPU
        spine.FillBoneConstraints();
    }
    else if (EditMode == emEditVolume && InputState.MouseLIsDown)
        MouseLMove_BVH(X, Y);
    else
    if (EditMode == emSelectVertex)
    {
        // show vertex info on hover
        std::vector<xDWORD> *sel = SelectCommon(X-3, Y-3, 6, 6);
        // if there were vertices inside the selection box
        if (sel) {
            std::vector<xDWORD>::iterator iter = sel->begin();
            while (iter != sel->end() && *iter >= Selection.Element->I_vertices)
                ++iter;
            HoveredVert = (iter != sel->end()) ? *iter : (xDWORD)-1;
            delete sel;
        }
        else
            HoveredVert = (xDWORD)-1;
    }
    else
    if (EditMode == emAnimateBones && InputState.MouseLIsDown && State.CurrentAction == IC_BE_Move)
    {
        xSkeleton &spine = Model.ModelGr->xModelP->Spine;
        if (Selection.Bone && Selection.Bone != spine.L_bones) // anim-rotate bone (matrix)
        {
            bool useVerlet = false;
            if (!useVerlet)
            {
                Selection.Bone->QT_rotation = Animation.Skeleton.PreviousQuaternion;
                Model.CalculateSkeleton();
                xMatrix  MX_bone  = Model.ModelGr->instance.MX_bones[Selection.Bone->ID]; // get current bone matrix
                xVector3 P_begin  = MX_bone.postTransformP(Selection.Bone->P_begin);  // get parent skeletized position
                xVector3 P_end    = MX_bone.postTransformP(Selection.Bone->P_end);    // get ending skeletized position
                xVector3 P_root   = CastPoint(P_begin, P_end);                        // get root on the current ending plane

                xVector3 P_hit = Cameras.Current->FOV.Get3dPos(X, Height-Y, P_end) - P_root; // get hit position relative to root, on the current ending plane
                P_end -= P_root;                                 // get ending relative to root

                float    W_cosF = xVector3::DotProduct(P_end.normalize(), P_hit.normalize()); // get cos of angle between ending and hit position
                float    W_angleH = acos(W_cosF)/2.0f;                                   // get angle between ending and hit position
                float    W_sinH   = sin(W_angleH);                                       // get sin of this angle
                xVector3 N_axis   = xVector3::CrossProduct(P_end, P_hit);                // get axis of rotation ending->hit position (relative to parent)
                N_axis = (MX_bone.invert() * N_axis).normalize();                        // transform axis to the current coordinates
                Selection.Bone->QT_rotation = xQuaternion::Product(                      // get rotation quaternion
                    Selection.Bone->QT_rotation,
                    xQuaternion::Create(N_axis * W_sinH, cos(W_angleH)) );
            }
            else
            {
                Model.CalculateSkeleton();

                vSystem.Free();
                vSystem.Init(spine.I_bones);
                vSystem.C_lengthConst = spine.C_boneLength;
                vSystem.I_constraints = spine.I_constraints;
                vSystem.C_constraints = spine.C_constraints;
                vSystem.Spine = &spine;
                vEngine.Init(vSystem);
                vEngine.I_passes = 10;

                xBone       *bone    = spine.L_bones;
                xVector3    *P_cur   = vSystem.P_current, *P_old = vSystem.P_previous;
                xQuaternion *QT_skew = vSystem.QT_boneSkew;
                xFLOAT      *M_iter  = vSystem.M_weight_Inv;
                xMatrix     *MX_bone = Model.ModelGr->instance.MX_bones;
                xQuaternion *QT_bone = Model.ModelGr->instance.QT_bones;
                for (int i = spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++MX_bone, ++QT_bone, ++M_iter)
                {
                    *P_cur  = *P_old = MX_bone->postTransformP(bone->P_end);
                    *QT_skew = bone->getSkew(*QT_bone);
                    *M_iter = 1 / bone->M_weight;
                }

                vSystem.M_weight_Inv[0] = 0.f;
                vSystem.P_current[Selection.Bone->ID] = Cameras.Current->FOV.Get3dPos(X, Height-Y, vSystem.P_current[Selection.Bone->ID]);
                vEngine.SatisfyConstraints();

                spine.CalcQuats(vSystem.P_current, vSystem.QT_boneSkew, 0, xMatrix::Identity());
            }
            Model.CalculateSkeleton();                                     // refresh model in GPU
        }
        else if (spine.L_bones) // anim-translate root bone (matrix)
        {
            Selection.Bone = spine.L_bones;
            xVector3 P_pos = Cameras.Current->FOV.Get3dPos(X, Height-Y, Selection.Bone->P_end);
            P_pos -= Selection.Bone->P_end;
            Selection.Bone->QT_rotation.init(P_pos, 1);
            Model.CalculateSkeleton();                                   // refresh model in GPU
        }
    }
    else
    if (EditMode == emLoadAnimation || EditMode == emSaveAnimation || EditMode == emSaveModel)
    {
        std::vector<GLButton>::iterator begin = Directories.begin();
        std::vector<GLButton>::iterator end   = Directories.end();
        for (; begin != end; ++begin)
            begin->Hover((xFLOAT)X, (xFLOAT)Y);
    }

    InputState.LastX = X;
    InputState.LastY = Y;
}

/***************************** 3D **************************************/
xVector3 SceneSkeleton::CastPoint(xPoint3 P_pointToCast, xPoint3 P_onPlane)
{
    // get plane of ray intersection
    xPlane PN_plane; PN_plane.init(
                        (Cameras.Current->P_center - Cameras.Current->P_eye).normalize(),
                        P_onPlane);
    return PN_plane.castPoint(P_pointToCast);
}

/************************* CONSTRAINTS *********************************/
void SceneSkeleton::GetConstraintParams()
{
    InputMgr &im = g_InputMgr;
    if (im.GetInputStateAndClear(IC_Accept))
    {
        float len = 0.f;
        if (InputState.String.length())
            len = atof (InputState.String.c_str());
        else
            len = Constraint.angles[Constraint.step];

        VConstraint *constr = NULL;
        if (Constraint.type == IC_BE_CreateConstrAng)
        {
            Constraint.angles[Constraint.step] = len;
            ++Constraint.step;
            InputState.String.clear();
            if (Constraint.step < 4)
            {
                Constraint.angles[Constraint.step] = len;
                return;
            }

            VConstraintAngular *res = new VConstraintAngular();
            xBone *bone = Model.ModelGr->xModelP->Spine.L_bones + Constraint.boneA;
            xBone *root;
            if (bone->ID_parent != 0)
            {
                root = Model.ModelGr->xModelP->Spine.L_bones + bone->ID_parent;
                res->particleRootB = root->ID_parent;
                res->particleRootE = root->ID;
            }
            else
            {
                root = Model.ModelGr->xModelP->Spine.L_bones;
                if (bone->ID == root->ID_kids[0])
                    root = Model.ModelGr->xModelP->Spine.L_bones + root->ID_kids[1];
                else
                    root = Model.ModelGr->xModelP->Spine.L_bones + root->ID_kids[0];
                res->particleRootB = root->ID;
                res->particleRootE = 0;
            }
            res->particle      = Constraint.boneA;

            res->angleMaxX = PI * Constraint.maxX / 180;
            res->angleMaxY = PI * Constraint.maxY / 180;
            res->angleMinX = PI * Constraint.minX / 180;
            res->angleMinY = PI * Constraint.minY / 180;
            res->minZ = res->maxZ = 0;
            constr = res;
        }
        else
            if (len <= 0.f)
                len = Constraint.S_length;
        if (Constraint.type == IC_BE_CreateConstrWeight)
        {
            xBone *bone = Model.ModelGr->xModelP->Spine.L_bones + Constraint.boneA;
            bone->M_weight = len;
            EditMode = emCreateBone;
            return;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrEql)
        {
            VConstraintLengthEql *res = new VConstraintLengthEql();
            res->particleA     = Constraint.boneA;
            res->particleB     = Constraint.boneB;
            res->restLength    = len;
            res->restLengthSqr = len*len;
            constr = res;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrMin)
        {
            VConstraintLengthMin *res = new VConstraintLengthMin();
            res->particleA     = Constraint.boneA;
            res->particleB     = Constraint.boneB;
            res->minLength    = len;
            res->minLengthSqr = len*len;
            constr = res;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrMax)
        {
            VConstraintLengthMax *res = new VConstraintLengthMax();
            res->particleA     = Constraint.boneA;
            res->particleB     = Constraint.boneB;
            res->maxLength    = len;
            res->maxLengthSqr = len*len;
            constr = res;
        }

        EditMode = emCreateBone;

        if (!constr) return;

        xSkeleton &spine = Model.ModelGr->xModelP->Spine;
        if (!spine.I_constraints)
        {
            spine.I_constraints = 1;
            spine.C_constraints = new VConstraint*[1];
            spine.C_constraints[0] = constr;
        }
        else
        {
            VConstraint** C_tmp = new VConstraint*[spine.I_constraints+1];
            memcpy (C_tmp, spine.C_constraints, spine.I_constraints*sizeof(VConstraint*));
            C_tmp[spine.I_constraints] = constr;
            delete[] spine.C_constraints;
            spine.C_constraints = C_tmp;
            ++spine.I_constraints;
        }
        if (Model.ModelPh)
            RigidObj::CopySpine(Model.ModelGr->xModelP->Spine, Model.ModelPh->xModelP->Spine);
    }
    else
        GetCommand();
}

/************************** SKININIG ***********************************/
void SceneSkeleton::GetBoneIdAndWeight()
{
    InputMgr &im = g_InputMgr;
    if (im.GetInputStateAndClear(IC_Accept))
    {
        int wght = 100;
        if (InputState.String.length())
            wght = atoi (InputState.String.c_str());

        if (wght > 0) {
            int sum = Skin.BoneWeight[0].weight +  Skin.BoneWeight[1].weight + Skin.BoneWeight[2].weight;
            if (wght > 100 - sum || Skin.BoneWeight[2].weight != 0)
                wght = 100 - sum;
            sum += wght;

            for (int i = 0; i < 4; ++i)
                if (Skin.BoneWeight[i].weight == 0)
                {

                    Skin.BoneWeight[i].id     = Selection.Bone->ID;
                    Skin.BoneWeight[i].weight = wght;
                    break;
                }
            if (sum >= 100) {
                EditMode = emSelectVertex;

                xDWORD stride = Selection.Element->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
                for (int dataSrc = 0; dataSrc < 2; ++dataSrc)
                {
                    xBYTE *verts = (xBYTE *) (dataSrc ? Selection.Element->L_vertices : Selection.Element->renderData.L_vertices);
                    xDWORD count = dataSrc ? Selection.Element->I_vertices : Selection.Element->renderData.I_vertices;
                    std::vector<xDWORD>::iterator iter = Selection.Vertices.begin();
                    for (; iter != Selection.Vertices.end(); ++iter)
                    {
                        if (*iter >= count) continue;
                        xVertexSkel *v = (xVertexSkel*)(verts + stride * *iter);
                        for (int i = 0; i < 4; ++i)
                            v->bone[i] = Skin.BoneWeight[i].id + Skin.BoneWeight[i].weight * 0.001f;
                    }
                }
                Model.VerticesChanged(false, true);
            }
            else {
                EditMode = emSelectBone;
                Selection.Bone = NULL;
            }
        }
    }
    else
        GetCommand();
}

/************************** ANIMATION **********************************/
void SceneSkeleton::GetFrameParams()
{
    InputMgr &im = g_InputMgr;
    if (im.GetInputStateAndClear(IC_Accept))
    {
        xLONG &wght = Animation.KeyFrame.step == 1 ? Animation.KeyFrame.freeze : Animation.KeyFrame.duration;
        if (InputState.String.length())
            wght = atoi (InputState.String.c_str());
        ++Animation.KeyFrame.step;
        if (Animation.KeyFrame.step > 2)
        {
            Animation.Instance->CurrentFrame->T_freeze   = Animation.KeyFrame.freeze;
            Animation.Instance->CurrentFrame->T_duration = Animation.KeyFrame.duration;
            EditMode = emEditAnimation;
        }
        InputState.String.clear();
    }
    else
        GetCommand();
}
void SceneSkeleton::GetCommand()
{
    InputMgr &im = g_InputMgr;
    if (im.GetInputStateAndClear(IC_Con_BackSpace))
    {
        if (InputState.String.length())
            InputState.String.erase(InputState.String.end()-1);
    }
    else if (im.Buffer.length())
    {
        InputState.String += im.Buffer;
        im.Buffer.clear();
    }
    im.mouseWheel = 0;
}
void SceneSkeleton::FillDirectoryBtns(bool files, const char *mask)
{
    Filesystem::Vec_string dirs = Filesystem::GetDirectories(CurrentDirectory);
    Directories.clear();
    xFLOAT top  = 5.f, left = 5.f, width = Width*0.333f-10;
    for (size_t i=0; i<dirs.size(); ++i)
    {
        top += 20;
        if (top > Height-45) { top = 25.f; left += Width*0.333f; }
        Directories.push_back(GLButton(dirs[i].c_str(), left, top, width, 15, IC_BE_Move));
    }
    if (files)
    {
        dirs = Filesystem::GetFiles(CurrentDirectory, mask);
        for (size_t i=0; i<dirs.size(); ++i)
        {
            top += 20;
            if (top > Height-45) { top = 25.f; left += Width*0.333f; }
            Directories.push_back(GLButton(dirs[i].c_str(), left, top, width, 15, IC_BE_Select));
            GLButton &btn = Directories.back();
            btn.Background.r = btn.Background.g = btn.Background.b = 0.35f;
        }
    }
}
/*************************** CAMERA ************************************/
void SceneSkeleton::UpdateDisplay(float deltaTime)
{
    InputMgr &im = g_InputMgr;

    if (im.GetInputStateAndClear(IC_PolyModeChange))
        Config::PolygonMode = (Config::PolygonMode == GL_FILL) ? GL_LINE : GL_FILL;

    if (im.GetInputStateAndClear(IC_CameraChange))
    {
        if (Cameras.Current == &Cameras.Front) Cameras.Current = &Cameras.Right;
        else
        if (Cameras.Current == &Cameras.Right) Cameras.Current = &Cameras.Back;
        else
        if (Cameras.Current == &Cameras.Back) Cameras.Current = &Cameras.Left;
        else
        if (Cameras.Current == &Cameras.Left) Cameras.Current = &Cameras.Top;
        else
        if (Cameras.Current == &Cameras.Top) Cameras.Current = &Cameras.Bottom;
        else
        if (Cameras.Current == &Cameras.Bottom) Cameras.Current = &Cameras.Perspective;
        else
        if (Cameras.Current == &Cameras.Perspective) Cameras.Current = &Cameras.Front;
    }
    if (im.GetInputStateAndClear(IC_CameraReset))
        InitCameras(true);
    if (im.GetInputStateAndClear(IC_CameraFront))
        Cameras.Current = &Cameras.Front;
    if (im.GetInputStateAndClear(IC_CameraBack))
        Cameras.Current = &Cameras.Back;
    if (im.GetInputStateAndClear(IC_CameraLeft))
        Cameras.Current = &Cameras.Left;
    if (im.GetInputStateAndClear(IC_CameraRight))
        Cameras.Current = &Cameras.Right;
    if (im.GetInputStateAndClear(IC_CameraTop))
        Cameras.Current = &Cameras.Top;
    if (im.GetInputStateAndClear(IC_CameraBottom))
        Cameras.Current = &Cameras.Bottom;
    if (im.GetInputStateAndClear(IC_CameraPerspective))
        Cameras.Current = &Cameras.Perspective;

    float scale = (Cameras.Current->P_eye - Cameras.Current->P_center).length();
    float run = (im.GetInputState(IC_RunModifier)) ? MULT_RUN : 1.0f;
    float deltaTmp = deltaTime * scale * run;

    if (im.GetInputState(IC_MoveForward))
        Cameras.Current->Move (deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_MoveBack))
        Cameras.Current->Move (-deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_MoveLeft))
        Cameras.Current->Move (0.0f, -deltaTmp, 0.0f);
    if (im.GetInputState(IC_MoveRight))
        Cameras.Current->Move (0.0f, deltaTmp, 0.0f);
    if (im.GetInputState(IC_MoveUp))
        Cameras.Current->Move (0.0f, 0.0f, deltaTmp);
    if (im.GetInputState(IC_MoveDown))
        Cameras.Current->Move (0.0f, 0.0f, -deltaTmp);
    if (im.mouseWheel != 0)
    {
        float scl = im.mouseWheel/180.0f;
        if (scl < 0.0) scl = -1/scl;
        Cameras.Current->P_eye = Cameras.Current->P_center + (Cameras.Current->P_eye - Cameras.Current->P_center)*scl;
        im.mouseWheel = 0;
    }

    if (Cameras.Current == &Cameras.Perspective)
    {
        deltaTmp = deltaTime*MULT_ROT*run;
        xVector3 center = Cameras.Current->P_center;

        if (im.GetInputState(IC_TurnLeft))
            Cameras.Current->Rotate (deltaTmp, 0.0f, 0.0f);
        if (im.GetInputState(IC_TurnRight))
            Cameras.Current->Rotate (-deltaTmp, 0.0f, 0.0f);
        if (im.GetInputState(IC_TurnUp))
            Cameras.Current->Rotate (0.0f, deltaTmp, 0.0f);
        if (im.GetInputState(IC_TurnDown))
            Cameras.Current->Rotate (0.0f, -deltaTmp, 0.0f);
        if (im.GetInputState(IC_RollLeft))
            Cameras.Current->Rotate (0.0f, 0.0f, -deltaTmp);
        if (im.GetInputState(IC_RollRight))
            Cameras.Current->Rotate (0.0f, 0.0f, deltaTmp);

        if (im.GetInputState(IC_OrbitLeft)) {
            Cameras.Current->P_center = center;
            Cameras.Current->Orbit (deltaTmp, 0.0f);
        }
        if (im.GetInputState(IC_OrbitRight)) {
            Cameras.Current->P_center = center;
            Cameras.Current->Orbit (-deltaTmp, 0.0f);
        }
        if (im.GetInputState(IC_OrbitUp)) {
            Cameras.Current->P_center = center;
            Cameras.Current->Orbit (0.0f, deltaTmp);
        }
        if (im.GetInputState(IC_OrbitDown)) {
            Cameras.Current->P_center = center;
            Cameras.Current->Orbit (0.0f, -deltaTmp);
        }
    }

    Cameras.Current->Update(deltaTime);
}

/**************************** BVH **************************************/
void SceneSkeleton::MouseLDown_BVH(int X, int Y)
{
    BVH.P_prevMouse = Cameras.Current->FOV.Get3dPos(X, Height-Y, Selection.BVHNode->Figure->P_center);
    xFLOAT S_dist = (BVH.P_prevMouse - Selection.BVHNode->Figure->P_center).lengthSqr();
    Selection.FigureDim = 0;

    if (g_InputMgr.GetInputState(IC_BE_Modifier))
        return;

    if (Selection.BVHNode->Figure->Type == xIFigure3d::Sphere)
        Selection.FigureDim = 1;
    else
    if (Selection.BVHNode->Figure->Type == xIFigure3d::Capsule)
    {
        const xCapsule &object = *(xCapsule*) Selection.BVHNode->Figure;

        xVector3 P_topCap = object.P_center + object.S_top * object.N_top;
        xVector3 P_mouse2 = Cameras.Current->FOV.Get3dPos(g_InputMgr.mouseX, Height-g_InputMgr.mouseY, P_topCap);
        xFLOAT   S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

        Selection.FigureDim = 3;

        if (S_dist2 < S_dist)
        {
            Selection.FigureDim = 1;
            BVH.P_prevMouse = P_mouse2;
        }
        else
        {
            P_topCap = object.P_center - object.S_top * object.N_top;
            P_mouse2 = Cameras.Current->FOV.Get3dPos(g_InputMgr.mouseX, Height-g_InputMgr.mouseY, P_topCap);
            S_dist2  = (P_mouse2 - P_topCap).lengthSqr();
            if (S_dist2 < S_dist)
            {
                Selection.FigureDim = 2;
                BVH.P_prevMouse = P_mouse2;
            }
        }
    }
    else
    if (Selection.BVHNode->Figure->Type == xIFigure3d::BoxOriented)
    {
        const xBoxO &object = *(xBoxO*) Selection.BVHNode->Figure;

        xVector3 P_topCap = object.P_center + object.S_top * object.N_top;
        xVector3 P_mouse2 = Cameras.Current->FOV.Get3dPos(g_InputMgr.mouseX, Height-g_InputMgr.mouseY, P_topCap);
        xFLOAT   S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

        if (S_dist2 < S_dist)
        {
            S_dist = S_dist2;
            Selection.FigureDim = 3;
            BVH.P_prevMouse = P_mouse2;
        }

        P_topCap = object.P_center + object.S_side * object.N_side;
        P_mouse2 = Cameras.Current->FOV.Get3dPos(g_InputMgr.mouseX, Height-g_InputMgr.mouseY, P_topCap);
        S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

        if (S_dist2 < S_dist)
        {
            S_dist = S_dist2;
            Selection.FigureDim = 2;
            BVH.P_prevMouse = P_mouse2;
        }

        P_topCap = object.P_center + object.S_front * object.N_front;
        P_mouse2 = Cameras.Current->FOV.Get3dPos(g_InputMgr.mouseX, Height-g_InputMgr.mouseY, P_topCap);
        S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

        if (S_dist2 < S_dist)
        {
            S_dist = S_dist2;
            Selection.FigureDim = 1;
            BVH.P_prevMouse = P_mouse2;
        }
    }
}

void SceneSkeleton::MouseLMove_BVH(int X, int Y)
{
    if (Selection.FigureDim == 0)
    {
        xPoint3 P_currMouse = Cameras.Current->FOV.Get3dPos(X, Height-Y, Selection.BVHNode->Figure->P_center);
        xVector3 NW_shift = P_currMouse - BVH.P_prevMouse;
        Selection.BVHNode->Figure->P_center += NW_shift;
        BVH.P_prevMouse = P_currMouse;
    }
    else
    if (Selection.BVHNode->Figure->Type == xIFigure3d::Sphere)
    {
        xSphere &object = *(xSphere*) Selection.BVHNode->Figure;
        xPoint3 P_mouse2 = Cameras.Current->FOV.Get3dPos(X, Height-Y, Selection.BVHNode->Figure->P_center);
        object.S_radius = (P_mouse2 - Selection.BVHNode->Figure->P_center).length();
    }
    else
    if (Selection.BVHNode->Figure->Type == xIFigure3d::Capsule)
    {
        xCapsule &object = *(xCapsule*) Selection.BVHNode->Figure;
        if (Selection.FigureDim == 1)
        {
            xPoint3 P_topCap = object.P_center + object.S_top * object.N_top;
            xPoint3 P_mouse2 = Cameras.Current->FOV.Get3dPos(X, Height-Y, P_topCap);
            xVector3 NW_shift = P_mouse2 - object.P_center;
            object.S_top = NW_shift.length();
            if (g_InputMgr.GetInputState(IC_RunModifier))
                object.N_top = NW_shift / object.S_top;
        }
        else
        if (Selection.FigureDim == 2)
        {
            xPoint3 P_topCap = object.P_center - object.S_top * object.N_top;
            xPoint3 P_mouse2 = Cameras.Current->FOV.Get3dPos(X, Height-Y, P_topCap);
            xVector3 NW_shift = object.P_center - P_mouse2;
            object.S_top = NW_shift.length();
            if (g_InputMgr.GetInputState(IC_RunModifier))
                object.N_top = NW_shift / object.S_top;
        }
        else
        {
            xPoint3 P_mouse2 = Cameras.Current->FOV.Get3dPos(X, Height-Y, Selection.BVHNode->Figure->P_center);
            object.S_radius = (P_mouse2 - Selection.BVHNode->Figure->P_center).length();
        }
    }
    else
    if (Selection.BVHNode->Figure->Type == xIFigure3d::BoxOriented)
    {
        xBoxO &object = *(xBoxO*) Selection.BVHNode->Figure;

        xQuaternion QT_rotation; QT_rotation.zeroQ();

        if (Selection.FigureDim == 1)
        {
            xPoint3 P_topCap = object.P_center + object.S_front * object.N_front;
            xPoint3 P_mouse2 = Cameras.Current->FOV.Get3dPos(X, Height-Y, P_topCap);
            if (g_InputMgr.GetInputState(IC_RunModifier))
                QT_rotation = xQuaternion::GetRotation(object.N_front, P_mouse2-object.P_center);
            object.S_front = (P_mouse2 - object.P_center).length();
        }
        else
        if (Selection.FigureDim == 2)
        {
            xPoint3 P_topCap = object.P_center + object.S_side * object.N_side;
            xPoint3 P_mouse2 = Cameras.Current->FOV.Get3dPos(X, Height-Y, P_topCap);
            if (g_InputMgr.GetInputState(IC_RunModifier))
                QT_rotation = xQuaternion::GetRotation(object.N_side, P_mouse2-object.P_center);
            object.S_side = (P_mouse2 - object.P_center).length();
        }
        else
        {
            xPoint3 P_topCap = object.P_center + object.S_top * object.N_top;
            xPoint3 P_mouse2 = Cameras.Current->FOV.Get3dPos(X, Height-Y, P_topCap);
            if (g_InputMgr.GetInputState(IC_RunModifier))
                QT_rotation = xQuaternion::GetRotation(object.N_top, P_mouse2-object.P_center);
            object.S_top = (P_mouse2 - object.P_center).length();
        }
        object.N_top   = QT_rotation.rotate(object.N_top).normalize();
        object.N_side  = QT_rotation.rotate(object.N_side).normalize();
        object.N_front = QT_rotation.rotate(object.N_front).normalize();
    }
    Selection.BVHNode->invalidateTransformation();
}
