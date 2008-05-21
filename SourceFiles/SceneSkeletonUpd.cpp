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

    std::vector<GLButton>::iterator begin = m_Buttons[m_EditMode].begin();
    std::vector<GLButton>::iterator end   = m_Buttons[m_EditMode].end();
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
        if (m_EditMode == emSelectElement || m_EditMode == emCreateBone || m_EditMode == emSelectAnimation)
            m_EditMode = emMain;
        else
        if (m_EditMode == emCreateConstraint_Type)
            m_EditMode = emCreateBone;
        else
        if (m_EditMode == emCreateConstraint_Node)
            m_EditMode = emCreateConstraint_Type;
        else
        if (m_EditMode == emCreateConstraint_Length)
        {
            frameParams[1] = -1;
            m_EditMode = emCreateConstraint_Node;
        }
        else
        if (m_EditMode == emSelectVertex)
        {
            m_EditMode = emSelectElement;
            selectedElemID = xWORD_MAX;
        }
        else
        if (m_EditMode == emSelectBone)
            m_EditMode = emSelectVertex;
        else
        if (m_EditMode == emInputWght) {
            m_EditMode = emSelectBone;
            selectedBone = NULL;
        }
        else
        if (m_EditMode == emEditAnimation) {
            m_EditMode = emSelectAnimation;
            currentAnimation->Unload();
            delete currentAnimation;
            currentAnimation = NULL;
        }
        else
        if (m_EditMode == emAnimateBones ||
            m_EditMode == emFrameParams ||
            m_EditMode == emLoadAnimation||
            m_EditMode == emSaveAnimation) {
            m_Directories.clear();
            m_EditMode = emEditAnimation;
        }
        else {
            Scene *tmp = m_PrevScene;
            m_PrevScene = NULL;
            g_Application.SetCurrentScene(tmp);
        }
        return true;
    }

    UpdateMouse(deltaTime);

    if (m_EditMode == emSelectVertex && im.GetInputStateAndClear(IC_Accept)) {
        if (selectedVert.size()) {
            m_EditMode = emSelectBone;
            selectedBone = NULL;
            boneIds[0] = 0; boneWghts[0] = 0;
            boneIds[1] = 0; boneWghts[1] = 0;
            boneIds[2] = 0; boneWghts[2] = 0;
            boneIds[3] = 0; boneWghts[3] = 0;
            return true;
        }
    }
    if (m_EditMode != emSelectVertex && m_EditMode != emSelectBone && m_EditMode != emInputWght &&
        im.GetInputStateAndClear(IC_ViewPhysicalModel))
    {
        m_EditGraphical = !m_EditGraphical;
        m_Model.CopySpineToPhysical();
        m_Buttons[emMain][3].Down = !m_EditGraphical;
    }
    if (m_EditMode == emCreateConstraint_Length)
    {
        GetConstraintParams();
        return true;
    }
    if (m_EditMode == emInputWght)
    {
        GetInputWeight();
        return true;
    }
    if (m_EditMode == emFrameParams)
    {
        GetFrameParams();
        return true;
    }
    if (m_EditMode == emSaveAnimation)
    {
        GetCommand();
        return true;
    }
    if (m_EditMode == emEditAnimation && play)
    {
        if (im.GetInputState(IC_BE_Modifier))
            currentAnimation->progress += (xWORD)(deltaTime*100);
        else
            currentAnimation->progress += (xWORD)(deltaTime*1000);
        currentAnimation->UpdatePosition();
        if (! currentAnimation->frameCurrent)
            currentAnimation->frameCurrent = currentAnimation->frameP;
        currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
        m_Model.CalculateSkeleton();
    }

    if (m_EditMode == emEditAnimation)
    {
        if (im.GetInputState(IC_BE_Modifier))
        {
            bool update = false;
            if (! currentAnimation->frameCurrent)
                currentAnimation->frameCurrent = currentAnimation->frameP;
            if (im.GetInputState(IC_MoveLeft))
            { currentAnimation->progress -= 1; update = true; }
            if (im.GetInputState(IC_MoveRight))
            { currentAnimation->progress += 1; update = true; }
            if (im.GetInputStateAndClear(IC_MoveUp))
            {
                if (currentAnimation->frameCurrent->next)
                {
                    currentAnimation->frameCurrent = currentAnimation->frameCurrent->next;
                    currentAnimation->progress = 0;
                }
                else
                    currentAnimation->progress = currentAnimation->frameCurrent->freeze + currentAnimation->frameCurrent->duration;
                update = true;
            }
            if (im.GetInputStateAndClear(IC_MoveDown))
            {
                if (currentAnimation->progress)
                    currentAnimation->progress = 0;
                else
                if (currentAnimation->frameCurrent->prev)
                    currentAnimation->frameCurrent = currentAnimation->frameCurrent->prev;
                update = true;
            }

            if (update)
            {
                currentAnimation->UpdatePosition();
                if (! currentAnimation->frameCurrent)
                    currentAnimation->frameCurrent = currentAnimation->frameP;
                currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
                m_Model.CalculateSkeleton();
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
        std::vector<GLButton>::iterator begin = m_Buttons[m_EditMode].begin();
        std::vector<GLButton>::iterator end   = m_Buttons[m_EditMode].end();
        for (; begin != end; ++begin)
            if (begin->RadioBox)
                begin->Down = button.Action == begin->Action;
        currentAction = button.Action;
    }

    if (m_EditMode == emMain)
    {
        if (button.Action == IC_BE_ModeSkeletize) {
            m_EditMode = emCreateBone;
            UpdateButton(m_Buttons[emCreateBone][0]);
            selectedBone = m_Model.GetModelGr()->spine.boneP;
            m_Model.GetModelGr()->spine.ResetQ();
            m_Model.CalculateSkeleton();
        }
        else
        if (button.Action == IC_BE_ModeSkin)
            m_EditMode = emSelectElement;
        else
        if (button.Action == IC_BE_ModeAnimate)
            m_EditMode = emSelectAnimation;
        else
        if (button.Action == IC_BE_Select)
        {
            m_EditGraphical = !m_EditGraphical;
            m_Model.CopySpineToPhysical();
            button.Down = !m_EditGraphical;
        }
        else
        if (button.Action == IC_BE_Save)
        {
            m_Model.GetModelGr()->Save();
            m_Model.CopySpineToPhysical();
            m_Model.GetModelPh()->Save();
        }

        return true;
    }
    if (m_EditMode == emCreateBone)
    {
        if (button.Action == IC_BE_Delete && selectedBone)
        {
            m_Model.CopySpineToPhysical();
            xBYTE boneId = selectedBone->id;
            selectedBone = NULL;
            m_Model.GetModelGr()->BoneDelete(boneId);
            if (m_Model.GetModelPh()->spine.boneP != m_Model.GetModelGr()->spine.boneP)
                m_Model.GetModelPh()->BoneDelete(boneId);
            m_Model.VerticesChanged(true);
        }
        if (button.Action == IC_BE_CreateConstr)
            m_EditMode = emCreateConstraint_Type;
        return true;
    }
    if (m_EditMode == emCreateConstraint_Type)
    {
        if (button.Action == IC_BE_CreateConstrMax ||
            button.Action == IC_BE_CreateConstrMin ||
            button.Action == IC_BE_CreateConstrEql)
        {
            frameParams[0] = -1;
            frameParams[1] = -1;
            constrType = button.Action;
            m_EditMode = emCreateConstraint_Node;
        }
        return true;
    }
    if (m_EditMode == emSelectAnimation)
    {
        if (button.Action == IC_BE_Create)
        {
            currentAnimation  = new xAnimation();
            currentAnimation->Reset(m_Model.GetModelGr()->spine.boneC);
            currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
            m_Model.CalculateSkeleton();
            m_Buttons[emEditAnimation][5].Down = false;
            m_EditMode = emEditAnimation;
            m_AnimationName.clear();
        }
        else
        if (button.Action == IC_BE_Select)
        {
            FillDirectoryBtns(true, "*.ska");
            m_EditMode = emLoadAnimation;
        }
        return true;
    }
    if (m_EditMode == emEditAnimation)
    {
        if (button.Action == IC_BE_Play)
            button.Down = play = !button.Down;
        if (button.Action == IC_BE_Create)
        {
            currentAnimation->InsertKeyFrame();
            m_EditMode = emAnimateBones;
            m_Buttons[emAnimateBones][3].Down = showBonesOnAnim = true;
            UpdateButton(m_Buttons[emAnimateBones][0]);
        }
        if (button.Action == IC_BE_Edit)
        {
            m_EditMode = emAnimateBones;
            m_Buttons[emAnimateBones][3].Down = showBonesOnAnim = true;
            UpdateButton(m_Buttons[emAnimateBones][0]);
            currentAnimation->progress = 0;
            currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
            m_Model.CalculateSkeleton();
        }
        if (button.Action == IC_BE_Move)
        {
            currentAnimation->progress = 0;
            currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
            m_Model.CalculateSkeleton();
            m_EditMode = emFrameParams;
            g_InputMgr.Buffer.clear();
            command.clear();
            param = 0;
            frameParams[0] = currentAnimation->frameCurrent->freeze;
            frameParams[1] = currentAnimation->frameCurrent->duration;
        }
        if (button.Action == IC_BE_Delete)
        {
            currentAnimation->DeleteKeyFrame();
            if (! currentAnimation->frameC)
                currentAnimation->InsertKeyFrame();
            currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
            m_Model.CalculateSkeleton();
        }
        else
        if (button.Action == IC_BE_Loop)
        {
            m_Buttons[emEditAnimation][5].Down = !m_Buttons[emEditAnimation][5].Down;
            if (currentAnimation->frameP->prev)
            {
                currentAnimation->frameP->prev->next = NULL;
                currentAnimation->frameP->prev = NULL;
            }
            else
            {
                xKeyFrame *frameP = currentAnimation->frameP;
                while (frameP->next)
                    frameP = frameP->next;
                currentAnimation->frameP->prev = frameP;
                frameP->next = currentAnimation->frameP;
            }
        }
        else
        if (button.Action == IC_BE_Save)
        {
            FillDirectoryBtns(true, "*.ska");
            m_EditMode = emSaveAnimation;
            command.clear();
            g_InputMgr.Buffer.clear();
        }
        return true;
    }
    if (m_EditMode == emAnimateBones)
    {
        if (button.Action == IC_BE_ModeSkeletize)
            button.Down = showBonesOnAnim = !showBonesOnAnim;
        else
        if (button.Action == IC_BE_Delete && selectedBone)
        {
            selectedBone->quaternion.zeroQ();
            m_Model.CalculateSkeleton();
        }
        else
        if (button.Action == IC_BE_Save)
        {
            currentAnimation->frameCurrent->LoadFromSkeleton(m_Model.GetModelGr()->spine);
            m_EditMode = emEditAnimation;
        }
        else
        if (button.Action == IC_Reject)
        {
            currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
            m_Model.CalculateSkeleton();
            m_EditMode = emEditAnimation;
        }
        return true;
    }
    if (m_EditMode == emSaveAnimation)
    {
        if (button.Action == IC_BE_Move)
        {
            if (!strcmp(button.Text, ".."))
                m_CurrentDirectory = Filesystem::GetParentDir(m_CurrentDirectory);
            else
            {
                m_CurrentDirectory += "/";
                m_CurrentDirectory += button.Text;
            }
            FillDirectoryBtns(true, "*.ska");
            return true;
        }
        if (button.Action == IC_BE_Select)
        {
            command = button.Text;
        }
        if (button.Action == IC_Accept)
        {
            if (command.size())
                m_AnimationName = command;
            if (m_AnimationName.size())
            {
                m_AnimationName = Filesystem::ChangeFileExt(m_AnimationName, "ska");
                std::string filePath = m_CurrentDirectory + "/";
                filePath += m_AnimationName;
                currentAnimation->Save(filePath.c_str());
                m_EditMode = emEditAnimation;
            }
        }
        if (button.Action == IC_Reject)
        {
            m_Directories.clear();
            m_EditMode = emEditAnimation;
        }
        return true;
    }
    if (m_EditMode == emLoadAnimation)
    {
        if (button.Action == IC_BE_Move)
        {
            if (!strcmp(button.Text, ".."))
                m_CurrentDirectory = Filesystem::GetParentDir(m_CurrentDirectory);
            else
            {
                m_CurrentDirectory += "/";
                m_CurrentDirectory += button.Text;
            }
            FillDirectoryBtns(true, "*.ska");
            return true;
        }
        if (button.Action == IC_BE_Select)
        {
            m_AnimationName = button.Text;
            std::string filePath = m_CurrentDirectory + "/";
            filePath += m_AnimationName;
            currentAnimation = new xAnimation();
            currentAnimation->Load(filePath.c_str());
            currentAnimation->SaveToSkeleton(m_Model.GetModelGr()->spine);
            m_Model.CalculateSkeleton();
            m_EditMode = emEditAnimation;
            m_Buttons[emEditAnimation][5].Down = currentAnimation->frameP && currentAnimation->frameP->prev;
        }
        if (button.Action == IC_Reject)
        {
            m_Directories.clear();
            m_EditMode = emSelectAnimation;
        }
        return true;
    }
    return false;
}

/************************** MOUSE **************************************/
void SceneSkeleton::UpdateMouse(float deltaTime)
{
    InputMgr &im = g_InputMgr;

    if (lastX != im.mouseX || lastY != im.mouseY)
        MouseMove(im.mouseX, im.mouseY);

    if (!mouseLIsDown && im.GetInputState(IC_LClick)) {
        mouseLIsDown = true;
        MouseLDown(im.mouseX, im.mouseY);
        return;
    }

    if (mouseLIsDown && !im.GetInputState(IC_LClick)) {
        mouseLIsDown = false;
        MouseLUp(im.mouseX, im.mouseY);
    }

    if (!mouseRIsDown && im.GetInputState(IC_RClick)) {
        mouseRIsDown = true;
        MouseRDown(im.mouseX, im.mouseY);
        return;
    }

    if (mouseRIsDown && !im.GetInputState(IC_RClick)) {
        mouseRIsDown = false;
        MouseRUp(im.mouseX, im.mouseY);
    }
}

void SceneSkeleton::MouseLDown(int X, int Y)
{
    if (m_EditMode == emSelectVertex)
    {
        selStartX = X;
        selStartY = Y;
    }
    else if (m_EditMode == emAnimateBones && selectedBone)
        lastBoneQuaternion = selectedBone->quaternion;
}

void SceneSkeleton::MouseLUp(int X, int Y)
{
    std::vector<GLButton>::iterator begin = m_Buttons[m_EditMode].begin();
    std::vector<GLButton>::iterator end   = m_Buttons[m_EditMode].end();
    for (; begin != end; ++begin)
        if (begin->Click(X, Height-Y) && UpdateButton(*begin))
            return;

    if (m_EditMode == emCreateBone)
    {
        if (currentAction == IC_BE_Create) // create bone
        {
            xVector3 hitPos = selectedBone
                ? selectedBone->pointE : xVector3::Create(0.0f, 0.0f, 0.0f);
            hitPos = Get3dPos(X, Y, hitPos);

            if (!m_Model.GetModelGr()->spine.boneC)                   // if no spine
            {
                m_Model.GetModelGr()->SkeletonAdd();                  //   add skeleton to model
                m_Model.GetModelPh()->SkeletonAdd();                  //   add skeleton to model
                selectedBone = m_Model.GetModelGr()->spine.boneP;     //   select root
                selectedBone->pointE = hitPos;                        //   set root position
            }
            else
            {
                if (!selectedBone) selectedBone = m_Model.GetModelGr()->spine.boneP;
                selectedBone = m_Model.GetModelGr()->spine.BoneAdd(selectedBone->id, hitPos); // add bone to skeleton
                m_Model.CopySpineToPhysical();
            }
            m_Model.CalculateSkeleton();
        }
        else
        if (currentAction == IC_BE_DeleteConstr)
        {
            std::vector<xDWORD> *sel = SelectCommon(X, Y);
            if (sel)
            {
                if (sel->size())
                {
                    int selectedConstr = sel->back();
                    xSkeleton &spine = m_Model.GetModelGr()->spine;
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
                    m_Model.CopySpineToPhysical();
                }
                delete sel;
            }
        }
        else
        if (currentAction != IC_BE_Move) // select bone
        {
            selectedBone = SelectBone(X,Y);
            if (!selectedBone) selectedBone = m_Model.GetModelGr()->spine.boneP;
        }
    }
    else
    if (m_EditMode == emCreateConstraint_Node)
    {
        xIKNode *bone = SelectBone(X,Y);
        if (!bone) bone = m_Model.GetModelGr()->spine.boneP;
        if (frameParams[0] == -1)
            frameParams[0] = bone->id;
        else
        if (frameParams[0] != bone->id)
        {
            frameParams[1] = bone->id;

            xIKNode *bone1 = m_Model.GetModelGr()->spine.boneP + frameParams[0];

            constrLen = (bone1->pointE - bone->pointE).length();
            m_EditMode = emCreateConstraint_Length;
            command.clear();
            g_InputMgr.Buffer.clear();
        }
    }
    else
    if (m_EditMode == emSelectElement) // select element
    {
        selectedElemID = (int) SelectElement(X,Y);
        if (selectedElemID != xWORD_MAX) { // MAXUINT
            if (m_EditGraphical)
                selectedElement = m_Model.GetModelGr()->kidsP->ById(selectedElemID);
            else
                selectedElement = m_Model.GetModelPh()->kidsP->ById(selectedElemID);
            m_EditMode = emSelectVertex;
            selectedVert.clear();
        }
        else
            selectedElement = NULL;
    }
    else
    if (m_EditMode == emSelectVertex) // select vertices inside the selection box
    {
        // normalize the selection box
        int selEndX = max(selStartX, X); selStartX = min(selStartX, X);
        int selEndY = max(selStartY, Y); selStartY = min(selStartY, Y);
        std::vector<xDWORD> *sel = SelectCommon(selStartX, selStartY,
                                                selEndX-selStartX, selEndY-selStartY);
        // if there were vertices inside the selection box
        if (sel) {
            std::vector<xDWORD>::iterator iter, found;
            for (iter = sel->begin(); iter < sel->end(); ++iter)
            {
                found = std::find<std::vector<xDWORD>::iterator, xDWORD> // check if vertex is selected
                                        (selectedVert.begin(), selectedVert.end(), *iter);

                if (!g_InputMgr.GetInputState(IC_BE_Modifier)) // if selecting
                {
                    if (found == selectedVert.end())       //   if not selected yet
                        selectedVert.push_back(*iter);     //     add vertex to selection
                }
                else                                       // if unselecting
                    if (found != selectedVert.end())       //   if selected
                        selectedVert.erase(found);         //     remove vertex from selection
            }
            delete sel;
        }
    }
    else
    if (m_EditMode == emSelectBone) // select bone and switch to the Input Weight Mode
    {
        selectedBone = SelectBone(X,Y);
        if (!selectedBone) selectedBone = m_Model.GetModelGr()->spine.boneP;

        m_EditMode = emInputWght;
        g_InputMgr.Buffer.clear();
        command.clear();
    }
    else
    if (m_EditMode == emAnimateBones && currentAction != IC_BE_Move) // select bone
    {
        selectedBone = SelectBone(X,Y);
        if (!selectedBone) selectedBone = m_Model.GetModelGr()->spine.boneP;
        UpdateButton(m_Buttons[emAnimateBones][1]); // switch to move mode
    }
    else
    if (m_EditMode == emLoadAnimation || m_EditMode == emSaveAnimation)
    {
        begin = m_Directories.begin();
        end   = m_Directories.end();
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
    if (m_EditMode == emSelectVertex)
        selectedVert.clear();
}

void CalcQuats(xIKNode *bones, const xVector3 *pos, xBYTE boneId, xMatrix parentMtxInv)
{
    xIKNode &bone = bones[boneId];
    xVector3 endN = parentMtxInv.postTransformP(pos[boneId]);

    if (boneId)
    {
        bone.quaternion = xQuaternion::getRotation(bone.pointE, endN, bone.pointB);
        xVector4 quat;
        quat.init(-bone.quaternion.vector3, bone.quaternion.w);

        parentMtxInv.preMultiply(xMatrixFromQuaternion(quat).preTranslate(bone.pointB).postTranslate(-bone.pointB));
    }
    else
    {
        bone.quaternion.init(endN - bone.pointE, 1.f);
        parentMtxInv = xMatrixTranslateT(bone.quaternion.vector3.xyz);
    }

    for (int i = 0; i < bone.joinsEC; ++i)
        CalcQuats(bones, pos, bone.joinsEP[i], parentMtxInv);
}

void SceneSkeleton::MouseMove(int X, int Y)
{
    std::vector<GLButton>::iterator begin = m_Buttons[m_EditMode].begin();
    std::vector<GLButton>::iterator end   = m_Buttons[m_EditMode].end();
    for (; begin != end; ++begin)
        begin->Hover(X, Height-Y);

    if (m_EditMode == emCreateBone && selectedBone && // edit-move bone (ending)
        mouseLIsDown && currentAction == IC_BE_Move)
    {
        selectedBone->pointE = Get3dPos(X, Y, selectedBone->pointE);
        xIKNode *nodes = m_Model.GetModelGr()->spine.boneP;
        xBYTE   *join  = selectedBone->joinsEP;
        for (xBYTE i = selectedBone->joinsEC; i; --i, ++join)
        {
            xIKNode &node = nodes[*join];
            node.pointB = selectedBone->pointE;
            node.curLengthSq = node.maxLengthSq = node.minLengthSq = (node.pointE-node.pointB).lengthSqr();
        }
        m_Model.CalculateSkeleton();                           // refresh model in GPU
    }
    else
    if (m_EditMode == emSelectVertex)
    {
        // show vertex info on hover
        std::vector<xDWORD> *sel = SelectCommon(X-3, Y-3, 6, 6);
        // if there were vertices inside the selection box
        if (sel) {
            std::vector<xDWORD>::iterator iter = sel->begin();
            while (iter != sel->end() && *iter >= selectedElement->verticesC)
                ++iter;
            hoveredVert = (iter != sel->end()) ? *iter : (xDWORD)-1;
            delete sel;
        }
        else
            hoveredVert = (xDWORD)-1;
    }
    else
    if (m_EditMode == emAnimateBones && mouseLIsDown && currentAction == IC_BE_Move)
    {
        xSkeleton &spine = m_Model.GetModelGr()->spine;
        if (selectedBone && selectedBone != spine.boneP) // anim-rotate bone (matrix)
        {
            //selectedBone->quaternion = lastBoneQuaternion;
            m_Model.CalculateSkeleton();

            //xMatrix  skel   = m_Model.modelInstanceGr.bonesM[selectedBone->id]; // get current bone matrix
            //xVector3 root   = skel.postTransformP(selectedBone->pointB); // get parent skeletized position
            //xVector3 ending = skel.postTransformP(selectedBone->pointE); // get ending skeletized position
            //xVector3 rootC  = CastPoint(root, ending);                   // get root on the current ending plane

            //xVector3 hitPos = Get3dPos(X, Y, ending) - rootC; // get hit position relative to root, on the current ending plane
            //ending -= rootC;                                  // get ending relative to root

            /*
            float cosF = xVector3::DotProduct(ending.normalize(), hitPos.normalize()); // get cos of angle between ending and hit position
            float angleH  = acos(cosF)/2.0f;                                 // get angle between ending and hit position
            float sinH    = sin(angleH);                                     // get sin of this angle
            xVector3 axis = xVector3::CrossProduct(ending, hitPos);          // get axis of rotation ending->hit position (relative to parent)
            axis = (skel.invert() * axis).normalize();                       // transform axis to the current coordinates
            selectedBone->quaternion = xQuaternion::product(                 // get rotation quaternion
                selectedBone->quaternion,
                xVector4::Create(axis.x*sinH, axis.y*sinH, axis.z*sinH, cos(angleH)) );
            */
            //IKEngine ikEngine;
            //ikEngine.Calculate(spine.boneP, spine.boneC, m_Model.modelInstanceGr.bonesM);

            engine.Free();
            engine.Init(spine.boneC);
            engine.m_numPasses = 10;

            xIKNode *bone = spine.boneP;
            xVector3 *pos = engine.m_pos, *posOld = engine.m_posOld;
            xMatrix  *mtx = m_Model.modelInstanceGr.bonesM;
            for (int i = spine.boneC; i; --i, ++bone, ++pos, ++posOld, ++mtx)
            {
                *pos = *posOld = mtx->postTransformP(bone->pointE);
                if (bone->joinsBC)
                {
                    xVConstraintLengthEql constraint;
                    constraint.particleA = bone->id;
                    constraint.particleB = bone->joinsBP[0];
                    constraint.restLengthSqr = bone->curLengthSq;
                    constraint.restLength = sqrt(bone->curLengthSq);
                    engine.lengthConstraints.push_back(constraint);
                }
            }
            engine.m_numConstraints = spine.constraintsC;
            engine.m_constraints = spine.constraintsP;

            engine.m_weight[0] = 0.f;
            engine.m_weight[selectedBone->id] = 0.f;
            engine.m_pos[selectedBone->id] = Get3dPos(X, Y, engine.m_pos[selectedBone->id]);
            engine.SatisfyConstraints();

            CalcQuats(spine.boneP, engine.m_pos, 0, xMatrix::Identity());
            m_Model.CalculateSkeleton();                                     // refresh model in GPU

            engine.m_constraints    = NULL;
            engine.m_numConstraints = 0;
        }
        else if (spine.boneP) // anim-translate root bone (matrix)
        {
            selectedBone = spine.boneP;
            xVector3 pos = Get3dPos(X, Y, selectedBone->pointE);
            pos -= selectedBone->pointE;
            selectedBone->quaternion.init(pos, 1);
            m_Model.CalculateSkeleton();                                   // refresh model in GPU
        }
    }
    else
    if (m_EditMode == emLoadAnimation || m_EditMode == emSaveAnimation)
    {
        std::vector<GLButton>::iterator begin = m_Directories.begin();
        std::vector<GLButton>::iterator end   = m_Directories.end();
        for (; begin != end; ++begin)
            begin->Hover(X, Height-Y);
    }

    lastX = X;
    lastY = Y;
}

/*************************** 3D ****************************************/
xVector3 SceneSkeleton::CastPoint(xVector3 rayPos, xVector3 planeP)
{
    // get plane of ray intersection
    xVector3 planeN = (m_Cameras.Current->center - m_Cameras.Current->eye).normalize();
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
    m_Cameras.Current->LookAtMatrix(modelView);
    modelView.invert();

    // get ray of the mouse
    xVector3 rayDir;
    xVector3 rayPos;
    if (m_Cameras.Current == &m_Cameras.Perspective)
    {
        float near_height = 0.1f * tan(45.0f * PI / 360.0f);
        rayPos = (xVector4::Create(0.0f,0.0f,0.0f,1.0f)*modelView).vector3;
        rayDir.init(near_height * AspectRatio * norm_x, near_height * norm_y, 0.1f);
    }
    else
    {
        double scale = fabs((m_Cameras.Current->eye - m_Cameras.Current->center).length());
        rayPos = (xVector4::Create(-scale *AspectRatio * norm_x,-scale * norm_y,0.0f,1.0f)*modelView).vector3;
        rayDir.init(0.f, 0.f, 0.1f);
    }
    rayDir = rayDir * modelView;

    // get plane of ray intersection
    xVector3 planeN = (m_Cameras.Current->center - m_Cameras.Current->eye).normalize();
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
        m_EditMode = emCreateBone;

        float len = 0.f;
        if (command.length())
            len = atof (command.c_str());
        if (len <= 0.f)
            len = constrLen;

        xIVConstraint *constr;
        if (constrType == IC_BE_CreateConstrEql)
        {
            xVConstraintLengthEql *res = new xVConstraintLengthEql();
            res->particleA     = frameParams[0];
            res->particleB     = frameParams[1];
            res->restLength    = len;
            res->restLengthSqr = len*len;
            constr = res;
        }
        else
        if (constrType == IC_BE_CreateConstrMin)
        {
            xVConstraintLengthMin *res = new xVConstraintLengthMin();
            res->particleA    = frameParams[0];
            res->particleB    = frameParams[1];
            res->minLength    = len;
            res->minLengthSqr = len*len;
            constr = res;
        }
        else
        if (constrType == IC_BE_CreateConstrMax)
        {
            xVConstraintLengthMax *res = new xVConstraintLengthMax();
            res->particleA    = frameParams[0];
            res->particleB    = frameParams[1];
            res->maxLength    = len;
            res->maxLengthSqr = len*len;
            constr = res;
        }
        else
            return;

        xSkeleton &spine = m_Model.GetModelGr()->spine;
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
        m_Model.CopySpineToPhysical();
    }
    else
        GetCommand();
}

/************************* SKININIG ************************************/
void SceneSkeleton::GetInputWeight()
{
    InputMgr &im = g_InputMgr;
    if (im.GetInputStateAndClear(IC_Accept))
    {
        int wght = 100;
        if (command.length())
            wght = atoi (command.c_str());

        if (wght > 0) {
            int sum = boneWghts[0] + boneWghts[1] + boneWghts[2];
            if (wght > 100 - sum) wght = 100 - sum;
            if (boneWghts[0] == 0) {
                boneIds[0] = selectedBone->id;
                boneWghts[0] = wght;
            }
            else
            if (boneWghts[1] == 0) {
                boneIds[1] = selectedBone->id;
                boneWghts[1] = wght;
            }
            else
            if (boneWghts[2] == 0) {
                boneIds[2] = selectedBone->id;
                boneWghts[2] = wght;
            }
            else
            if (boneWghts[3] == 0) {
                boneIds[3] = selectedBone->id;
                boneWghts[3] = wght = 100 - sum;
            }
            sum += wght;
            if (sum >= 100) {
                m_EditMode = emSelectVertex;
                SetVertexWghts();
                //selectedVert.clear();
            }
            else {
                m_EditMode = emSelectBone;
                selectedBone = NULL;
            }
        }
    }
    else
        GetCommand();
}
void SceneSkeleton::SetVertexWghts()
{
    if (selectedElement)
    {
        if (!selectedElement->skeletized) return;

        xDWORD stride = selectedElement->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        for (int dataSrc = 0; dataSrc < 2; ++dataSrc)
        {
            xBYTE *verts = (xBYTE *) (dataSrc ? selectedElement->verticesP : selectedElement->renderData.verticesP);
            xDWORD count = dataSrc ? selectedElement->verticesC : selectedElement->renderData.verticesC;
            std::vector<xDWORD>::iterator iter = selectedVert.begin();
            for (; iter != selectedVert.end(); ++iter)
            {
                if (*iter >= count) continue;
                xVertexSkel *v = (xVertexSkel*)(verts + stride * *iter);
                v->b0 = boneIds[0] + boneWghts[0] / 1000.0f;
                v->b1 = boneIds[1] + boneWghts[1] / 1000.0f;
                v->b2 = boneIds[2] + boneWghts[2] / 1000.0f;
                v->b3 = boneIds[3] + boneWghts[3] / 1000.0f;
            }
        }
        m_Model.VerticesChanged(true);
    }
}

/************************* ANIMATION ***********************************/
void SceneSkeleton::GetFrameParams()
{
    InputMgr &im = g_InputMgr;
    if (im.GetInputStateAndClear(IC_Accept))
    {
        int wght = frameParams[param];
        if (command.length())
            wght = atoi (command.c_str());
        frameParams[param] = wght;
        ++param;
        if (param > 1)
        {
            currentAnimation->frameCurrent->freeze   = frameParams[0];
            currentAnimation->frameCurrent->duration = frameParams[1];
            m_EditMode = emEditAnimation;
        }
        command.clear();
    }
    else
        GetCommand();
}
void SceneSkeleton::GetCommand()
{
    InputMgr &im = g_InputMgr;
    if (im.GetInputStateAndClear(IC_Con_BackSpace))
    {
        if (command.length())
            command.erase(command.end()-1);
    }
    else if (im.Buffer.length())
    {
        command += im.Buffer;
        im.Buffer.clear();
    }
    im.mouseWheel = 0;
}
void SceneSkeleton::FillDirectoryBtns(bool files, const char *mask)
{
    Filesystem::VectorString dirs = Filesystem::GetDirectories(m_CurrentDirectory);
    m_Directories.clear();
    int top  = Height-20, left = 5, width = Width/3-10;
    for (size_t i=0; i<dirs.size(); ++i)
    {
        top -= 20;
        if (top < 45) { top = Height-40; left += Width/3; }
        m_Directories.push_back(GLButton(dirs[i].c_str(), left, top, width, 15, IC_BE_Move));
    }
    if (files)
    {
        dirs = Filesystem::GetFiles(m_CurrentDirectory, mask);
        for (size_t i=0; i<dirs.size(); ++i)
        {
            top -= 20;
            if (top < 45) { top = Height-40; left += Width/3; }
            m_Directories.push_back(GLButton(dirs[i].c_str(), left, top, width, 15, IC_BE_Select));
            GLButton &btn = m_Directories.back();
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
        if (m_Cameras.Current == &m_Cameras.Front) m_Cameras.Current = &m_Cameras.Right;
        else
        if (m_Cameras.Current == &m_Cameras.Right) m_Cameras.Current = &m_Cameras.Back;
        else
        if (m_Cameras.Current == &m_Cameras.Back) m_Cameras.Current = &m_Cameras.Left;
        else
        if (m_Cameras.Current == &m_Cameras.Left) m_Cameras.Current = &m_Cameras.Top;
        else
        if (m_Cameras.Current == &m_Cameras.Top) m_Cameras.Current = &m_Cameras.Bottom;
        else
        if (m_Cameras.Current == &m_Cameras.Bottom) m_Cameras.Current = &m_Cameras.Perspective;
        else
        if (m_Cameras.Current == &m_Cameras.Perspective) m_Cameras.Current = &m_Cameras.Front;
    }
    if (im.GetInputStateAndClear(IC_CameraReset))
    {
        if (m_Cameras.Current == &m_Cameras.Front)
            m_Cameras.Front.SetCamera(0.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (m_Cameras.Current == &m_Cameras.Right)
            m_Cameras.Right.SetCamera(-5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (m_Cameras.Current == &m_Cameras.Back)
            m_Cameras.Back.SetCamera(0.0f, +5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (m_Cameras.Current == &m_Cameras.Left)
            m_Cameras.Left.SetCamera(+5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (m_Cameras.Current == &m_Cameras.Top)
            m_Cameras.Top.SetCamera(0.0f, 0.0f,  5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        if (m_Cameras.Current == &m_Cameras.Bottom)
            m_Cameras.Bottom.SetCamera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        if (m_Cameras.Current == &m_Cameras.Perspective)
            m_Cameras.Perspective.SetCamera(-5.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
    }
    if (im.GetInputStateAndClear(IC_CameraFront))
        m_Cameras.Current = &m_Cameras.Front;
    if (im.GetInputStateAndClear(IC_CameraBack))
        m_Cameras.Current = &m_Cameras.Back;
    if (im.GetInputStateAndClear(IC_CameraLeft))
        m_Cameras.Current = &m_Cameras.Left;
    if (im.GetInputStateAndClear(IC_CameraRight))
        m_Cameras.Current = &m_Cameras.Right;
    if (im.GetInputStateAndClear(IC_CameraTop))
        m_Cameras.Current = &m_Cameras.Top;
    if (im.GetInputStateAndClear(IC_CameraBottom))
        m_Cameras.Current = &m_Cameras.Bottom;
    if (im.GetInputStateAndClear(IC_CameraPerspective))
        m_Cameras.Current = &m_Cameras.Perspective;

    float scale = (m_Cameras.Current->eye - m_Cameras.Current->center).length();
    float run = (im.GetInputState(IC_RunModifier)) ? MULT_RUN : 1.0f;
    float deltaTmp = deltaTime * scale * run;

    if (im.GetInputState(IC_MoveForward))
        m_Cameras.Current->Move (deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_MoveBack))
        m_Cameras.Current->Move (-deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_MoveLeft))
        m_Cameras.Current->Move (0.0f, -deltaTmp, 0.0f);
    if (im.GetInputState(IC_MoveRight))
        m_Cameras.Current->Move (0.0f, deltaTmp, 0.0f);
    if (im.GetInputState(IC_MoveUp))
        m_Cameras.Current->Move (0.0f, 0.0f, deltaTmp);
    if (im.GetInputState(IC_MoveDown))
        m_Cameras.Current->Move (0.0f, 0.0f, -deltaTmp);
    if (im.mouseWheel != 0)
    {
        float scl = im.mouseWheel/240.0f;
        if (scl < 0.0) scl = -1/scl;
        m_Cameras.Current->eye = m_Cameras.Current->center + (m_Cameras.Current->eye - m_Cameras.Current->center)*scl;
        im.mouseWheel = 0;
    }

    if (m_Cameras.Current == &m_Cameras.Perspective)
    {
        deltaTmp = deltaTime*MULT_ROT*run;
        xVector3 center = m_Cameras.Current->center;

        if (im.GetInputState(IC_TurnLeft))
            m_Cameras.Current->Rotate (deltaTmp, 0.0f, 0.0f);
        if (im.GetInputState(IC_TurnRight))
            m_Cameras.Current->Rotate (-deltaTmp, 0.0f, 0.0f);
        if (im.GetInputState(IC_TurnUp))
            m_Cameras.Current->Rotate (0.0f, deltaTmp, 0.0f);
        if (im.GetInputState(IC_TurnDown))
            m_Cameras.Current->Rotate (0.0f, -deltaTmp, 0.0f);
        if (im.GetInputState(IC_RollLeft))
            m_Cameras.Current->Rotate (0.0f, 0.0f, -deltaTmp);
        if (im.GetInputState(IC_RollRight))
            m_Cameras.Current->Rotate (0.0f, 0.0f, deltaTmp);

        if (im.GetInputState(IC_OrbitLeft)) {
            m_Cameras.Current->center = center;
            m_Cameras.Current->Orbit (deltaTmp, 0.0f);
        }
        if (im.GetInputState(IC_OrbitRight)) {
            m_Cameras.Current->center = center;
            m_Cameras.Current->Orbit (-deltaTmp, 0.0f);
        }
        if (im.GetInputState(IC_OrbitUp)) {
            m_Cameras.Current->center = center;
            m_Cameras.Current->Orbit (0.0f, deltaTmp);
        }
        if (im.GetInputState(IC_OrbitDown)) {
            m_Cameras.Current->center = center;
            m_Cameras.Current->Orbit (0.0f, -deltaTmp);
        }
    }
}
