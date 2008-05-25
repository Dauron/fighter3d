#include "SceneSkeleton.h"
#include "SceneConsole.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../Utils/Filesystem.h"
#include "../Physics/IKEngine.h"

#define MULT_MOVE   5.0f
#define MULT_RUN    2.0f
#define MULT_ROT    80.0f
#define MULT_STEP   60.0f

/************************** INPUT **************************************/
bool SceneSkeleton::Update(float deltaTime)
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
            Animation.Instance->progress += (xWORD)(deltaTime*100);
        else
            Animation.Instance->progress += (xWORD)(deltaTime*1000);
        Animation.Instance->UpdatePosition();
        if (! Animation.Instance->frameCurrent)
            Animation.Instance->frameCurrent = Animation.Instance->frameP;
        Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
        Model.CalculateSkeleton();
    }

    if (EditMode == emEditAnimation)
    {
        if (im.GetInputState(IC_BE_Modifier))
        {
            bool update = false;
            if (! Animation.Instance->frameCurrent)
                Animation.Instance->frameCurrent = Animation.Instance->frameP;
            if (im.GetInputState(IC_MoveLeft))
            { Animation.Instance->progress -= 1; update = true; }
            if (im.GetInputState(IC_MoveRight))
            { Animation.Instance->progress += 1; update = true; }
            if (im.GetInputStateAndClear(IC_MoveUp))
            {
                if (Animation.Instance->frameCurrent->next)
                {
                    Animation.Instance->frameCurrent = Animation.Instance->frameCurrent->next;
                    Animation.Instance->progress = 0;
                }
                else
                    Animation.Instance->progress = Animation.Instance->frameCurrent->freeze + Animation.Instance->frameCurrent->duration;
                update = true;
            }
            if (im.GetInputStateAndClear(IC_MoveDown))
            {
                if (Animation.Instance->progress)
                    Animation.Instance->progress = 0;
                else
                if (Animation.Instance->frameCurrent->prev)
                    Animation.Instance->frameCurrent = Animation.Instance->frameCurrent->prev;
                update = true;
            }

            if (update)
            {
                Animation.Instance->UpdatePosition();
                if (! Animation.Instance->frameCurrent)
                    Animation.Instance->frameCurrent = Animation.Instance->frameP;
                Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
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
            Selection.Bone = Model.GetModelGr()->spine.boneP;
            Model.GetModelGr()->spine.ResetQ();
            Model.CalculateSkeleton();
        }
        else // must be skeletized to perform skinning
        if (button.Action == IC_BE_ModeSkin && Model.GetModelGr()->spine.boneP)
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
            xBYTE boneId = Selection.Bone->id;
            Selection.Bone = NULL;
            Model.GetModelGr()->BoneDelete(boneId);
            if (Model.GetModelPh()->spine.boneP != Model.GetModelGr()->spine.boneP)
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
            Animation.Instance->Reset(Model.GetModelGr()->spine.boneC);
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
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
            Animation.Instance->progress = 0;
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
            Model.CalculateSkeleton();
        }
        if (button.Action == IC_BE_Move)
        {
            Animation.Instance->progress = 0;
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
            Model.CalculateSkeleton();
            EditMode = emFrameParams;
            g_InputMgr.Buffer.clear();
            InputState.String.clear();
            Animation.KeyFrame.step = 1;
            Animation.KeyFrame.freeze   = Animation.Instance->frameCurrent->freeze;
            Animation.KeyFrame.duration = Animation.Instance->frameCurrent->duration;
        }
        if (button.Action == IC_BE_Delete)
        {
            Animation.Instance->DeleteKeyFrame();
            if (! Animation.Instance->frameC)
                Animation.Instance->InsertKeyFrame();
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
            Model.CalculateSkeleton();
        }
        else
        if (button.Action == IC_BE_Loop)
        {
            Buttons[emEditAnimation][5].Down = !Buttons[emEditAnimation][5].Down;
            if (Animation.Instance->frameP->prev)
            {
                Animation.Instance->frameP->prev->next = NULL;
                Animation.Instance->frameP->prev = NULL;
            }
            else
            {
                xKeyFrame *frameP = Animation.Instance->frameP;
                while (frameP->next)
                    frameP = frameP->next;
                Animation.Instance->frameP->prev = frameP;
                frameP->next = Animation.Instance->frameP;
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
            Selection.Bone->quaternion.zeroQ();
            Model.CalculateSkeleton();
        }
        else
        if (button.Action == IC_BE_Save)
        {
            Animation.Instance->frameCurrent->LoadFromSkeleton(Model.GetModelGr()->spine);
            EditMode = emEditAnimation;
        }
        else
        if (button.Action == IC_Reject)
        {
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
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
            Animation.Instance->SaveToSkeleton(Model.GetModelGr()->spine);
            Model.CalculateSkeleton();
            EditMode = emEditAnimation;
            Buttons[emEditAnimation][5].Down = Animation.Instance->frameP && Animation.Instance->frameP->prev;
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
        Animation.PreviousQuaternion = Selection.Bone->quaternion;
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
                ? Selection.Bone->pointE : xVector3::Create(0.0f, 0.0f, 0.0f);
            hitPos = Get3dPos(X, Y, hitPos);

            if (!Model.GetModelGr()->spine.boneC)                   // if no spine
            {
                Model.GetModelGr()->SkeletonAdd();                  //   add skeleton to model
                Model.GetModelPh()->SkeletonAdd();                  //   add skeleton to model
                Selection.Bone = Model.GetModelGr()->spine.boneP;     //   select root
                Selection.Bone->pointE = hitPos;                        //   set root position
            }
            else
            {
                if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->spine.boneP;
                Selection.Bone = Model.GetModelGr()->spine.BoneAdd(Selection.Bone->id, hitPos); // add bone to skeleton
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
                    xSkeleton &spine = Model.GetModelGr()->spine;
                    if (spine.constraintsC == 1)
                    {
                        spine.constraintsC = 0;
                        delete *spine.constraintsP;
                        delete[] spine.constraintsP;
                        spine.constraintsP = NULL;
                    }
                    else
                    {
                        xIVConstraint** tmp = new xIVConstraint*[spine.constraintsC-1];
                        memcpy (tmp, spine.constraintsP, selectedConstr*sizeof(xIVConstraint*));
                        memcpy (tmp+selectedConstr, spine.constraintsP+selectedConstr+1, (spine.constraintsC-selectedConstr-1)*sizeof(xIVConstraint*));
                        delete spine.constraintsP[selectedConstr];
                        delete[] spine.constraintsP;
                        spine.constraintsP = tmp;
                        --spine.constraintsC;
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
            if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->spine.boneP;
        }
    }
    else
    if (EditMode == emCreateConstraint_Node)
    {
        xIKNode *bone = SelectBone(X,Y);
        xIKNode *root = Model.GetModelGr()->spine.boneP;
        if (!bone) bone = root;

        if (Constraint.type == IC_BE_CreateConstrAng)
        {
            if (bone->id == 0 || (bone->joinsBP[0] == 0 && root->joinsEC == 1 )) return;
            Constraint.boneA = bone->id;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrWeight)
            Constraint.boneA = bone->id;
        else
        if (Constraint.boneA == xBYTE_MAX)
            Constraint.boneA = bone->id;
        else
        if (Constraint.boneA != bone->id)
            Constraint.boneB = bone->id;

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
            Constraint.length = bone->weight;
            InputState.String.clear();
            g_InputMgr.Buffer.clear();
        }
        else if (Constraint.boneB != xBYTE_MAX)
        {
            xIKNode *bone1 = Model.GetModelGr()->spine.boneP + Constraint.boneA;
            Constraint.length = (bone1->pointE - bone->pointE).length();
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
                Selection.Element = Model.GetModelPh()->kidsP->ById(Selection.ElementId);
            else
                Selection.Element = Model.GetModelGr()->kidsP->ById(Selection.ElementId);
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
        if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->spine.boneP;

        EditMode = emInputWght;
        g_InputMgr.Buffer.clear();
        InputState.String.clear();
    }
    else
    if (EditMode == emAnimateBones && State.CurrentAction != IC_BE_Move) // select bone
    {
        Selection.Bone = SelectBone(X,Y);
        if (!Selection.Bone) Selection.Bone = Model.GetModelGr()->spine.boneP;
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
        Selection.Bone->pointE = Get3dPos(X, Y, Selection.Bone->pointE);
        Selection.Bone->curLengthSq = Selection.Bone->maxLengthSq = Selection.Bone->minLengthSq = (Selection.Bone->pointE-Selection.Bone->pointB).lengthSqr();
        xSkeleton &spine = Model.GetModelGr()->spine;
        xIKNode *nodes = spine.boneP;
        xBYTE   *join  = Selection.Bone->joinsEP;
        for (xBYTE i = Selection.Bone->joinsEC; i; --i, ++join)
        {
            xIKNode &node = nodes[*join];
            node.pointB = Selection.Bone->pointE;
            node.curLengthSq = node.maxLengthSq = node.minLengthSq = (node.pointE-node.pointB).lengthSqr();
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
            while (iter != sel->end() && *iter >= Selection.Element->verticesC)
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
        xSkeleton &spine = Model.GetModelGr()->spine;
        if (Selection.Bone && Selection.Bone != spine.boneP) // anim-rotate bone (matrix)
        {
            bool useVerlet = false;
            if (!useVerlet)
            {
                Selection.Bone->quaternion = Animation.PreviousQuaternion;
                Model.CalculateSkeleton();
                xMatrix  skel   = Model.modelInstanceGr.bonesM[Selection.Bone->id]; // get current bone matrix
                xVector3 root   = skel.postTransformP(Selection.Bone->pointB); // get parent skeletized position
                xVector3 ending = skel.postTransformP(Selection.Bone->pointE); // get ending skeletized position
                xVector3 rootC  = CastPoint(root, ending);                   // get root on the current ending plane

                xVector3 hitPos = Get3dPos(X, Y, ending) - rootC; // get hit position relative to root, on the current ending plane
                ending -= rootC;                                  // get ending relative to root

                float cosF = xVector3::DotProduct(ending.normalize(), hitPos.normalize()); // get cos of angle between ending and hit position
                float angleH  = acos(cosF)/2.0f;                                 // get angle between ending and hit position
                float sinH    = sin(angleH);                                     // get sin of this angle
                xVector3 axis = xVector3::CrossProduct(ending, hitPos);          // get axis of rotation ending->hit position (relative to parent)
                axis = (skel.invert() * axis).normalize();                       // transform axis to the current coordinates
                Selection.Bone->quaternion = xQuaternion::product(                 // get rotation quaternion
                    Selection.Bone->quaternion,
                    xVector4::Create(axis.x*sinH, axis.y*sinH, axis.z*sinH, cos(angleH)) );
                //IKEngine ikEngine;
                //ikEngine.Calculate(spine.boneP, spine.boneC, Model.modelInstanceGr.bonesM);
            }
            else
            {
                Model.CalculateSkeleton();

                vSystem.Free();
                vSystem.Init(spine.boneC);
                vSystem.C_lengthConst = spine.boneConstrP;
                vSystem.I_constraints = spine.constraintsC;
                vSystem.C_constraints = spine.constraintsP;
                vSystem.spine = &spine;
                vEngine.Init(&vSystem);
                vEngine.I_passes = 10;

                xIKNode  *bone = spine.boneP;
                xVector3 *P_cur = vSystem.P_current, *P_old = vSystem.P_previous;
                xFLOAT   *M_iter = vSystem.M_weight_Inv;
                xMatrix  *MX_bone = Model.modelInstanceGr.bonesM;
                for (int i = spine.boneC; i; --i, ++bone, ++P_cur, ++P_old, ++MX_bone, ++M_iter)
                {
                    *P_cur  = *P_old = MX_bone->postTransformP(bone->pointE);
                    *M_iter = 1 / bone->weight;
                }

                vSystem.M_weight_Inv[0] = 0.f;
                vSystem.P_current[Selection.Bone->id] = Get3dPos(X, Y, vSystem.P_current[Selection.Bone->id]);
                vEngine.SatisfyConstraints();

                spine.CalcQuats(vSystem.P_current, 0, xMatrix::Identity());
            }
            Model.CalculateSkeleton();                                     // refresh model in GPU
        }
        else if (spine.boneP) // anim-translate root bone (matrix)
        {
            Selection.Bone = spine.boneP;
            xVector3 pos = Get3dPos(X, Y, Selection.Bone->pointE);
            pos -= Selection.Bone->pointE;
            Selection.Bone->quaternion.init(pos, 1);
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
xVector3 SceneSkeleton::CastPoint(xVector3 rayPos, xVector3 planeP)
{
    // get plane of ray intersection
    xVector3 planeN = (Cameras.Current->center - Cameras.Current->eye).normalize();
    float planeD = -xVector3::DotProduct(planeN, planeP);
    float dist  = xVector3::DotProduct(planeN, rayPos) + planeD; // distance to plane
    return rayPos - planeN * dist;
}

xVector3 SceneSkeleton::Get3dPos(int X, int Y, xVector3 planeP)
{
    float norm_x = 1.0f - (float)X/(Width/2.0f);
    float norm_y = (float)Y/(Height/2.0f) - 1.0f;

    // get model view matrix
    xMatrix modelView;
    Cameras.Current->LookAtMatrix(modelView);
    modelView.invert();

    // get ray of the mouse
    xVector3 rayDir;
    xVector3 rayPos;
    if (Cameras.Current == &Cameras.Perspective)
    {
        float near_height = 0.1f * tan(45.0f * PI / 360.0f);
        rayPos = (xVector4::Create(0.0f,0.0f,0.0f,1.0f)*modelView).vector3;
        rayDir.init(near_height * AspectRatio * norm_x, near_height * norm_y, 0.1f);
    }
    else
    {
        double scale = fabs((Cameras.Current->eye - Cameras.Current->center).length());
        rayPos = (xVector4::Create(-scale *AspectRatio * norm_x,-scale * norm_y,0.0f,1.0f)*modelView).vector3;
        rayDir.init(0.f, 0.f, 0.1f);
    }
    rayDir = rayDir * modelView;

    // get plane of ray intersection
    xVector3 planeN = (Cameras.Current->center - Cameras.Current->eye).normalize();
    float planeD = -xVector3::DotProduct(planeN, planeP);

    const float a = xVector3::DotProduct(planeN, rayDir); // get cos between vectors
    if (a != 0) // if ray is not parallel to the plane (is not perpendicular to the plane normal)
    {
        float dist  = xVector3::DotProduct(planeN, rayPos) + planeD; // distance to plane
        return rayPos - rayDir * (dist / a);
    }
    return xVector3();
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

        xIVConstraint *constr;
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

            xVConstraintAngular *res = new xVConstraintAngular();
            xIKNode *bone = Model.GetModelGr()->spine.boneP + Constraint.boneA;
            xIKNode *root;
            if (bone->joinsBP[0] != 0)
            {
                root = Model.GetModelGr()->spine.boneP + bone->joinsBP[0];
                res->particleRootB = root->joinsBP[0];
                res->particleRootE = root->id;
                res->upQuat = xQuaternion::getRotation(root->pointE - root->pointB, bone->pointE - bone->pointB);
            }
            else
            {
                root = Model.GetModelGr()->spine.boneP;
                if (bone->id == root->joinsEP[0])
                    root = Model.GetModelGr()->spine.boneP + root->joinsEP[1];
                else
                    root = Model.GetModelGr()->spine.boneP + root->joinsEP[0];
                res->particleRootB = root->id;
                res->particleRootE = 0;
                res->upQuat = xQuaternion::getRotation(root->pointB - root->pointE, bone->pointE - bone->pointB);
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
                len = Constraint.length;
        if (Constraint.type == IC_BE_CreateConstrWeight)
        {
            xIKNode *bone = Model.GetModelGr()->spine.boneP + Constraint.boneA;
            bone->weight = len;
            EditMode = emCreateBone;
            return;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrEql)
        {
            xVConstraintLengthEql *res = new xVConstraintLengthEql();
            res->particleA     = Constraint.boneA;
            res->particleB     = Constraint.boneB;
            res->restLength    = len;
            res->restLengthSqr = len*len;
            constr = res;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrMin)
        {
            xVConstraintLengthMin *res = new xVConstraintLengthMin();
            res->particleA     = Constraint.boneA;
            res->particleB     = Constraint.boneB;
            res->minLength    = len;
            res->minLengthSqr = len*len;
            constr = res;
        }
        else
        if (Constraint.type == IC_BE_CreateConstrMax)
        {
            xVConstraintLengthMax *res = new xVConstraintLengthMax();
            res->particleA     = Constraint.boneA;
            res->particleB     = Constraint.boneB;
            res->maxLength    = len;
            res->maxLengthSqr = len*len;
            constr = res;
        }

        EditMode = emCreateBone;

        xSkeleton &spine = Model.GetModelGr()->spine;
        if (!spine.constraintsP)
        {
            spine.constraintsC = 1;
            spine.constraintsP = new xIVConstraint*[1];
            spine.constraintsP[0] = constr;
        }
        else
        {
            xIVConstraint** tmp = new xIVConstraint*[spine.constraintsC+1];
            memcpy (tmp, spine.constraintsP, (spine.constraintsC)*sizeof(xIVConstraint*));
            tmp[spine.constraintsC] = constr;
            delete[] spine.constraintsP;
            spine.constraintsP = tmp;
            ++spine.constraintsC;
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

                    Skin.BoneWeight[i].id     = Selection.Bone->id;
                    Skin.BoneWeight[i].weight = wght;
                    break;
                }
            if (sum >= 100) {
                EditMode = emSelectVertex;

                xDWORD stride = Selection.Element->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
                for (int dataSrc = 0; dataSrc < 2; ++dataSrc)
                {
                    xBYTE *verts = (xBYTE *) (dataSrc ? Selection.Element->verticesP : Selection.Element->renderData.verticesP);
                    xDWORD count = dataSrc ? Selection.Element->verticesC : Selection.Element->renderData.verticesC;
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
            Animation.Instance->frameCurrent->freeze   = Animation.KeyFrame.freeze;
            Animation.Instance->frameCurrent->duration = Animation.KeyFrame.duration;
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
