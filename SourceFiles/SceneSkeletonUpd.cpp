#include "SceneSkeleton.h"
#include "SceneConsole.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../Utils/Filesystem.h"
#include "../Physics/Verlet/VConstraintLengthEql.h"
#include "../Physics/Verlet/VConstraintLengthMin.h"
#include "../Physics/Verlet/VConstraintLengthMax.h"
#include "../Physics/Verlet/VConstraintAngular.h"

#define MULT_MOVE   5.0f
#define MULT_RUN    2.0f
#define MULT_ROT    80.0f
#define MULT_STEP   60.0f

/************************** INPUT **************************************/
bool SceneSkeleton::FrameUpdate(float deltaTime)
{
    InputMgr &im = g_InputMgr;

    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        if (im.GetInputStateAndClear(begin->Action))
        { UpdateButton(*begin); return true; }

    if (im.GetInputStateAndClear(IC_Console)) {
        g_Application.SetCurrentScene(new SceneConsole(this), false);
        return true;
    }
    if (im.GetInputStateAndClear(IC_FullScreen)) {
        g_Application.MainWindow().SetFullScreen(!g_Application.MainWindow().FullScreen());
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
            Scene *tmp = PrevScene;
            PrevScene = NULL;
            g_Application.SetCurrentScene(tmp);
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
    if (im.GetInputStateAndClear(IC_ShowBonesAlways))
        State.ShowBonesAlways = !State.ShowBonesAlways;
    if (EditMode != emSelectVertex && EditMode != emSelectBone && EditMode != emInputWght &&
        im.GetInputStateAndClear(IC_ViewPhysicalModel))
    {
        State.DisplayPhysical = !State.DisplayPhysical;
        Model.CopySpineToPhysical();
        Buttons[emMain][3].Down = State.DisplayPhysical;
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
    if (EditMode == emSaveAnimation)
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
        Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
        Model.CalculateSkeleton();
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
                Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
                Model.CalculateSkeleton();
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
            Selection.Bone = Model.GetModelGr()->Spine.L_bones;
            Model.GetModelGr()->Spine.ResetQ();
            Model.CalculateSkeleton();
        }
        else // must be skeletized to perform skinning
        if (button.Action == IC_BE_ModeSkin && Model.GetModelGr()->Spine.L_bones)
            EditMode = emSelectElement;
        else
        if (button.Action == IC_BE_ModeAnimate)
            EditMode = emSelectAnimation;
        else
        if (button.Action == IC_BE_Select)
        {
            State.DisplayPhysical = !State.DisplayPhysical;
            Model.CopySpineToPhysical();
            button.Down = State.DisplayPhysical;
        }
        else
        if (button.Action == IC_BE_Save)
        {
            Model.GetModelGr()->Save();
            Model.CopySpineToPhysical();
            Model.GetModelPh()->Save();
        }

        return true;
    }
    if (EditMode == emCreateBone)
    {
        if (button.Action == IC_BE_Delete && Selection.Bone)
        {
            Model.CopySpineToPhysical();
            xBYTE boneId = Selection.Bone->ID;
            Selection.Bone = NULL;
            Model.GetModelGr()->BoneDelete(boneId);
            if (Model.GetModelPh()->Spine.L_bones != Model.GetModelGr()->Spine.L_bones)
                Model.GetModelPh()->BoneDelete(boneId);
            Model.VerticesChanged(true);
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
    if (EditMode == emSelectAnimation)
    {
        if (button.Action == IC_BE_Create)
        {
            Animation.Instance  = new xAnimation();
            Animation.Instance->Reset(Model.GetModelGr()->Spine.I_bones);
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
            Model.CalculateSkeleton();
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
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
            Model.CalculateSkeleton();
        }
        if (button.Action == IC_BE_Move)
        {
            Animation.Instance->T_progress = 0;
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
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
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
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
            InputState.String.clear();
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
            Animation.Instance->CurrentFrame->LoadFromSkeleton(Model.GetModelGr()->Spine);
            EditMode = emEditAnimation;
        }
        else
        if (button.Action == IC_Reject)
        {
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
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
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->Spine);
            Model.CalculateSkeleton();
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

void SceneSkeleton::MouseLDown(int X, int Y)
{
    if (EditMode == emSelectVertex)
    {
        Selection.RectStartX = X;
        Selection.RectStartY = Y;
    }
    else if (EditMode == emAnimateBones && Selection.Bone)
        Animation.PreviousQuaternion = Selection.Bone->QT_rotation;
}

void SceneSkeleton::MouseLUp(int X, int Y)
{
    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        if (begin->Click(X, Height-Y) && UpdateButton(*begin))
            return;

    if (EditMode == emCreateBone)
    {
        if (State.CurrentAction == IC_BE_Create) // create bone
        {
            xVector3 hitPos = Selection.Bone
                ? Selection.Bone->P_end : xVector3::Create(0.0f, 0.0f, 0.0f);
            hitPos = Get3dPos(X, Y, hitPos);

            if (!Model.GetModelGr()->Spine.I_bones)                 // if no spine
            {
                Model.GetModelGr()->SkeletonAdd();                  //   add skeleton to model
                Model.GetModelPh()->SkeletonAdd();                  //   add skeleton to model
                Selection.Bone = Model.GetModelGr()->Spine.L_bones; //   select root
                Selection.Bone->P_end = hitPos;                     //   set root position
            }
            else
            {
                if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->Spine.L_bones;
                Selection.Bone = Model.GetModelGr()->Spine.BoneAdd(Selection.Bone->ID, hitPos); // add bone to skeleton
                Model.CopySpineToPhysical();
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
                    xSkeleton &spine = Model.GetModelGr()->Spine;
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
                    Model.CopySpineToPhysical();
                }
                delete sel;
            }
        }
        else
        if (State.CurrentAction != IC_BE_Move) // select bone
        {
            Selection.Bone = SelectBone(X,Y);
            if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->Spine.L_bones;
        }
    }
    else
    if (EditMode == emCreateConstraint_Node)
    {
        xBone *bone = SelectBone(X,Y);
        xBone *root = Model.GetModelGr()->Spine.L_bones;
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
            xBone *bone1 = Model.GetModelGr()->Spine.L_bones + Constraint.boneA;
            Constraint.S_length = bone1->S_length;
            EditMode = emCreateConstraint_Params;
            InputState.String.clear();
            g_InputMgr.Buffer.clear();
        }
    }
    else
    if (EditMode == emSelectElement) // select element
    {
        Selection.ElementId = (int) SelectElement(X,Y);
        if (Selection.ElementId != xWORD_MAX) { // MAXUINT
            if (State.DisplayPhysical)
                Selection.Element = Model.GetModelPh()->L_kids->ById(Selection.ElementId);
            else
                Selection.Element = Model.GetModelGr()->L_kids->ById(Selection.ElementId);
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
        if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->Spine.L_bones;

        EditMode = emInputWght;
        g_InputMgr.Buffer.clear();
        InputState.String.clear();
    }
    else
    if (EditMode == emAnimateBones && State.CurrentAction != IC_BE_Move) // select bone
    {
        Selection.Bone = SelectBone(X,Y);
        if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->Spine.L_bones;
        UpdateButton(Buttons[emAnimateBones][1]); // switch to move mode
    }
    else
    if (EditMode == emLoadAnimation || EditMode == emSaveAnimation)
    {
        begin = Directories.begin();
        end   = Directories.end();
        for (; begin != end; ++begin)
            if (begin->Click(X, Height-Y) && UpdateButton(*begin))
                return;
    }
}

void SceneSkeleton::MouseRDown(int X, int Y)
{
}
void SceneSkeleton::MouseRUp(int X, int Y)
{
    if (EditMode == emSelectVertex)
        Selection.Vertices.clear();
}


void SceneSkeleton::MouseMove(int X, int Y)
{
    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        begin->Hover(X, Height-Y);

    if (EditMode == emCreateBone && Selection.Bone && // edit-move bone (ending)
        InputState.MouseLIsDown && State.CurrentAction == IC_BE_Move)
    {
        Selection.Bone->P_end = Get3dPos(X, Y, Selection.Bone->P_end);
        Selection.Bone->S_lengthSqr = (Selection.Bone->P_end-Selection.Bone->P_begin).lengthSqr();
        Selection.Bone->S_length    = sqrt(Selection.Bone->S_lengthSqr);
        xSkeleton &spine = Model.GetModelGr()->Spine;
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
        xSkeleton &spine = Model.GetModelGr()->Spine;
        if (Selection.Bone && Selection.Bone != spine.L_bones) // anim-rotate bone (matrix)
        {
            bool useVerlet = false;
            if (!useVerlet)
            {
                Selection.Bone->QT_rotation = Animation.PreviousQuaternion;
                Model.CalculateSkeleton();
                xMatrix  MX_bone  = Model.modelInstanceGr.MX_bones[Selection.Bone->ID]; // get current bone matrix
                xVector3 P_begin  = MX_bone.postTransformP(Selection.Bone->P_begin);  // get parent skeletized position
                xVector3 P_end    = MX_bone.postTransformP(Selection.Bone->P_end);    // get ending skeletized position
                xVector3 P_root   = CastPoint(P_begin, P_end);                        // get root on the current ending plane

                xVector3 P_hit = Get3dPos(X, Y, P_end) - P_root; // get hit position relative to root, on the current ending plane
                P_end -= P_root;                                 // get ending relative to root

                float    W_cosF = xVector3::DotProduct(P_end.normalize(), P_hit.normalize()); // get cos of angle between ending and hit position
                float    W_angleH = acos(W_cosF)/2.0f;                                   // get angle between ending and hit position
                float    W_sinH   = sin(W_angleH);                                       // get sin of this angle
                xVector3 N_axis   = xVector3::CrossProduct(P_end, P_hit);                // get axis of rotation ending->hit position (relative to parent)
                N_axis = (MX_bone.invert() * N_axis).normalize();                        // transform axis to the current coordinates
                Selection.Bone->QT_rotation = xQuaternion::product(                      // get rotation quaternion
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
                xMatrix     *MX_bone = Model.modelInstanceGr.MX_bones;
                xQuaternion *QT_bone = Model.modelInstanceGr.QT_bones;
                for (int i = spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++MX_bone, QT_bone+=2, ++M_iter)
                {
                    *P_cur  = *P_old = MX_bone->postTransformP(bone->P_end);
                    *QT_skew = bone->getSkew(*QT_bone);
                    *M_iter = 1 / bone->M_weight;
                }

                vSystem.M_weight_Inv[0] = 0.f;
                vSystem.P_current[Selection.Bone->ID] = Get3dPos(X, Y, vSystem.P_current[Selection.Bone->ID]);
                vEngine.SatisfyConstraints();

                spine.CalcQuats(vSystem.P_current, vSystem.QT_boneSkew,
                                0, xMatrix::Identity(), xVector3::Create(0.f,0.f,0.f));
            }
            Model.CalculateSkeleton();                                     // refresh model in GPU
        }
        else if (spine.L_bones) // anim-translate root bone (matrix)
        {
            Selection.Bone = spine.L_bones;
            xVector3 P_pos = Get3dPos(X, Y, Selection.Bone->P_end);
            P_pos -= Selection.Bone->P_end;
            Selection.Bone->QT_rotation.init(P_pos, 1);
            Model.CalculateSkeleton();                                   // refresh model in GPU
        }
    }
    else
    if (EditMode == emLoadAnimation || EditMode == emSaveAnimation)
    {
        std::vector<GLButton>::iterator begin = Directories.begin();
        std::vector<GLButton>::iterator end   = Directories.end();
        for (; begin != end; ++begin)
            begin->Hover(X, Height-Y);
    }

    InputState.LastX = X;
    InputState.LastY = Y;
}

/*************************** 3D ****************************************/
xVector3 SceneSkeleton::CastPoint(xPoint3 P_pointToCast, xPoint3 P_onPlane)
{
    // get plane of ray intersection
    xPlane PN_plane; PN_plane.init(
                        (Cameras.Current->center - Cameras.Current->eye).normalize(),
                        P_onPlane);
    return PN_plane.castPoint(P_pointToCast);
}

xVector3 SceneSkeleton::Get3dPos(int X, int Y, xPoint3 P_onPlane)
{
    float norm_x = 1.0f - (float)X/(Width/2.0f);
    float norm_y = (float)Y/(Height/2.0f) - 1.0f;

    // get model view matrix
    xMatrix MX_ModelToView;
    Cameras.Current->LookAtMatrix(MX_ModelToView);
    MX_ModelToView.invert();

    // get ray of the mouse
    xVector3 N_ray;
    xVector3 P_ray;
    if (Cameras.Current == &Cameras.Perspective)
    {
        float near_height = 0.1f * tan(45.0f * PI / 360.0f);
        P_ray = (xVector4::Create(0.0f,0.0f,0.0f,1.0f)*MX_ModelToView).vector3;
        N_ray.init(near_height * AspectRatio * norm_x, near_height * norm_y, 0.1f);
    }
    else
    {
        double scale = fabs((Cameras.Current->eye - Cameras.Current->center).length());
        P_ray = (xVector4::Create(-scale *AspectRatio * norm_x,-scale * norm_y,0.0f,1.0f)*MX_ModelToView).vector3;
        N_ray.init(0.f, 0.f, 0.1f);
    }
    N_ray = MX_ModelToView.preTransformV(N_ray);

    // get plane of ray intersection
    xPlane PN_plane; PN_plane.init(
        (Cameras.Current->eye-Cameras.Current->center).normalize(), P_onPlane);
    return PN_plane.intersectRay(P_ray, N_ray);
}

/************************* CONSTRAINTS ************************************/
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

        VConstraint *constr;
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
            xBone *bone = Model.GetModelGr()->Spine.L_bones + Constraint.boneA;
            xBone *root;
            if (bone->ID_parent != 0)
            {
                root = Model.GetModelGr()->Spine.L_bones + bone->ID_parent;
                res->particleRootB = root->ID_parent;
                res->particleRootE = root->ID;
            }
            else
            {
                root = Model.GetModelGr()->Spine.L_bones;
                if (bone->ID == root->ID_kids[0])
                    root = Model.GetModelGr()->Spine.L_bones + root->ID_kids[1];
                else
                    root = Model.GetModelGr()->Spine.L_bones + root->ID_kids[0];
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
            xBone *bone = Model.GetModelGr()->Spine.L_bones + Constraint.boneA;
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

        xSkeleton &spine = Model.GetModelGr()->Spine;
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
        Model.CopySpineToPhysical();
    }
    else
        GetCommand();
}

/************************* SKININIG ************************************/
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
                Model.VerticesChanged(true);
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

/************************* ANIMATION ***********************************/
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
    Filesystem::VectorString dirs = Filesystem::GetDirectories(CurrentDirectory);
    Directories.clear();
    int top  = Height-20, left = 5, width = Width/3-10;
    for (size_t i=0; i<dirs.size(); ++i)
    {
        top -= 20;
        if (top < 45) { top = Height-40; left += Width/3; }
        Directories.push_back(GLButton(dirs[i].c_str(), left, top, width, 15, IC_BE_Move));
    }
    if (files)
    {
        dirs = Filesystem::GetFiles(CurrentDirectory, mask);
        for (size_t i=0; i<dirs.size(); ++i)
        {
            top -= 20;
            if (top < 45) { top = Height-40; left += Width/3; }
            Directories.push_back(GLButton(dirs[i].c_str(), left, top, width, 15, IC_BE_Select));
            GLButton &btn = Directories.back();
            btn.Background.r = btn.Background.g = btn.Background.b = 0.35f;
        }
    }
}
/************************** CAMERA *************************************/
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
    {
        if (Cameras.Current == &Cameras.Front)
            Cameras.Front.SetCamera(0.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Right)
            Cameras.Right.SetCamera(-5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Back)
            Cameras.Back.SetCamera(0.0f, +5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Left)
            Cameras.Left.SetCamera(+5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Top)
            Cameras.Top.SetCamera(0.0f, 0.0f,  5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        if (Cameras.Current == &Cameras.Bottom)
            Cameras.Bottom.SetCamera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        if (Cameras.Current == &Cameras.Perspective)
            Cameras.Perspective.SetCamera(-5.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
    }
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

    float scale = (Cameras.Current->eye - Cameras.Current->center).length();
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
        float scl = im.mouseWheel/240.0f;
        if (scl < 0.0) scl = -1/scl;
        Cameras.Current->eye = Cameras.Current->center + (Cameras.Current->eye - Cameras.Current->center)*scl;
        im.mouseWheel = 0;
    }

    if (Cameras.Current == &Cameras.Perspective)
    {
        deltaTmp = deltaTime*MULT_ROT*run;
        xVector3 center = Cameras.Current->center;

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
            Cameras.Current->center = center;
            Cameras.Current->Orbit (deltaTmp, 0.0f);
        }
        if (im.GetInputState(IC_OrbitRight)) {
            Cameras.Current->center = center;
            Cameras.Current->Orbit (-deltaTmp, 0.0f);
        }
        if (im.GetInputState(IC_OrbitUp)) {
            Cameras.Current->center = center;
            Cameras.Current->Orbit (0.0f, deltaTmp);
        }
        if (im.GetInputState(IC_OrbitDown)) {
            Cameras.Current->center = center;
            Cameras.Current->Orbit (0.0f, -deltaTmp);
        }
    }
}
