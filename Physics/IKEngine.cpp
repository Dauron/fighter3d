#include "IKEngine.h"

void IKEngine :: Calculate(xIKNode *nodeArray, xBYTE nodeCount, xMatrix *boneM)
{
    if (!nodeCount) return;

    // make dirty
    xIKNode *node = nodeArray;
    for (int i=nodeCount; i; --i, ++node, ++boneM)
    {
        node->checked = false;
        node->modified = false;
        node->pointBT = boneM->postTransformP(node->pointB);
        node->pointET = boneM->postTransformP(node->pointE);
    }
    boneM -= nodeCount;

    ProcessNode(nodeArray, 0, boneM);
}

void IKEngine :: ProcessNode(xIKNode *nodeArray, xBYTE nodeId, xMatrix *boneM)
{
    xIKNode &node = nodeArray[nodeId];
    if (node.checked) return;

    node.checked = true;

    xQuaternion quat; quat.zeroQ();

    if (node.forcesValid && node.destination != node.pointET)
    {
        node.forcesValid = false;
        quat = xQuaternion::getRotation(node.pointET, node.destination, node.pointBT);
        node.pointET    = xQuaternion::rotate(quat, node.pointET, node.pointBT);
        node.quaternion = xQuaternion::product(node.quaternion, quat);
        if (node.joinsBC)
            boneM[node.id] = boneM[node.joinsBP[0]] *
                xMatrixFromQuaternion(node.quaternion).preTranslate(node.pointB).postTranslate(-node.pointB);
        else
            boneM[node.id] =
                xMatrixFromQuaternion(node.quaternion).preTranslate(node.pointB).postTranslate(-node.pointB);
    }

    node.modified = quat.w < 0.99f;
    if (node.modified)
        PropagateChangesForward(nodeArray, node.id, boneM, node.pointB);

    xBYTE *join = node.joinsEP;
    for (xBYTE i=node.joinsEC; i; --i, ++join)
        ProcessNode(nodeArray, *join, boneM);
}

void IKEngine :: PropagateChangesForward(xIKNode *nodeArray, xBYTE nodeId, xMatrix *boneM, const xVector3 &changeRoot)
{
    xIKNode &node = nodeArray[nodeId];

    xVector3 nodeAxis = node.pointET - changeRoot;
    xFLOAT   nodeDist = nodeAxis.length();
    if (nodeDist != 0.f) nodeAxis /= nodeDist;

    xBYTE *join = node.joinsEP;
    for (xBYTE i=node.joinsEC; i; --i, ++join)
    {
        xIKNode &next = nodeArray[*join];

        boneM[next.id] = boneM[node.id] *
            xMatrixFromQuaternion(next.quaternion).preTranslate(next.pointB).postTranslate(-next.pointB);
        xVector3 oldPointE = boneM[next.id].postTransformP(next.pointE);
        xVector3 newPointE;

        xVector3 slideAxis     = (next.pointET - changeRoot).normalize();
        xFLOAT   cosSlideAngle = xVector3::DotProduct(slideAxis, nodeAxis);
        xFLOAT   cSlideDist    = nodeDist * cosSlideAngle;
        xVector3 slideCenter   = changeRoot + slideAxis * cSlideDist;

        xFLOAT   dist = (slideCenter - node.pointET).lengthSqr();

        if (dist < next.curLengthSq)
        {
            xFLOAT slideDist = sqrtf(next.curLengthSq - dist);
            newPointE = slideCenter + slideAxis*slideDist;
        }
        else
        {
            slideAxis = (slideCenter - node.pointET).normalize();
            newPointE = node.pointET + slideAxis*sqrtf(next.curLengthSq);
        }

        xQuaternion quat = xQuaternion::getRotation(oldPointE, newPointE, next.pointB);
        next.pointET     = xQuaternion::rotate(quat, next.pointET, next.pointBT);
        next.quaternion  = xQuaternion::product(next.quaternion, quat);
        boneM[next.id]   = boneM[node.id] *
            xMatrixFromQuaternion(next.quaternion).preTranslate(next.pointB).postTranslate(-next.pointB);

        PropagateChangesForward(nodeArray, next.id, boneM, changeRoot);
    }
}
