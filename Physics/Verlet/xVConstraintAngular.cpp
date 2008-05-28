#include "xVConstraintAngular.h"
#include "xVerletSolver.h"
#include "../../Models/lib3dx/xSkeleton.h"

bool xVConstraintAngular :: Satisfy(xVerletSystem *system)
{
    xVector3 &P_rootB = system->P_current[particleRootB];
    xVector3 &P_rootE = system->P_current[particleRootE];
    xVector3 &P_curr  = system->P_current[particle];

    xVector4 QT_parent, QT_current;
    // OPTIMIZE: calc only needed quats
    system->spine->CalcQuats(system->P_current, 0, system->MX_WorldToModel_T);
    xBoneCalculateQuatForVerlet(*system->spine, particle, QT_parent, QT_current);
    QT_current = xQuaternion::product(QT_parent, QT_current);

    xBone   &bone    = system->spine->L_bones[particle];
    xVector3 N_up    = system->MX_ModelToWorld.preTransformV(xQuaternion::rotate(QT_parent, bone.P_end-bone.P_begin)).normalize();
    xVector3 N_front = system->MX_ModelToWorld.preTransformV(xQuaternion::rotate(QT_parent, xVector3::Create(0,1,0)));

    xMatrix  MX_WorldToBone = xMatrixFromVectors( N_front, N_up, - P_rootE ).invert();
    xVector3 P_curr_Local   = MX_WorldToBone.preTransformP( P_curr );
    xVector3 N_curr_Local   = xVector3::Normalize( P_curr_Local );

    xFLOAT alpha   = atan2(fabs(P_curr_Local.y), fabs(P_curr_Local.x));
    xFLOAT scale   = 2*alpha * PI_inv;
    xFLOAT betaMax;

    if (P_curr_Local.x >= 0 && P_curr_Local.y >= 0)
        betaMax = angleMaxY * scale + angleMaxX * (1.f - scale);
    else if (P_curr_Local.x < 0 && P_curr_Local.y >= 0)
        betaMax = angleMaxY * scale + angleMinX * (1.f - scale);
    else if (P_curr_Local.x >= 0 && P_curr_Local.y < 0)
        betaMax = angleMinY * scale + angleMaxX * (1.f - scale);
    else
        betaMax = angleMinY * scale + angleMinX * (1.f - scale);

    xFLOAT cosMax = cos(betaMax);
    if (N_curr_Local.z >= cosMax) return false;

    xVector3 P_new_Local;
    xFLOAT xy     = sin(betaMax);
    P_new_Local.x = xy*cos(alpha);
    P_new_Local.y = xy*sin(alpha);
    P_new_Local.z = cosMax;

    xVector4 quat = xQuaternion::getRotation(N_curr_Local, P_new_Local);

    xFLOAT w1 = system->M_weight_Inv[particle];
    xFLOAT w2 = system->M_weight_Inv[particleRootE];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    xVector4 quatP = xQuaternion::interpolate(quat, w1);
    xVector4 quatR = xQuaternion::interpolate(quat, w2);
    xMatrix &MX_BoneToWorld = MX_WorldToBone.invert();
    P_rootE = MX_BoneToWorld.preTransformP( xQuaternion::rotate(quatR, -P_curr_Local) + P_curr_Local );
    P_curr  = MX_BoneToWorld.preTransformP( xQuaternion::rotate(quatP, P_curr_Local) );

    return true;
}
bool xVConstraintAngular :: Test(const xVector3 &P_rootB, const xVector3 &P_rootE, const xVector3 &P_curr,
                                 const xVector3 &N_up, const xVector3 &N_front) const
{
    xMatrix MX_WorldToBone = xMatrixFromVectors(N_front, N_up, -P_rootE).invert();
    xVector3 P_curr_Local  = MX_WorldToBone.preTransformP( P_curr );
    xVector3 N_curr_Local  = xVector3::Normalize( P_curr_Local );

    xFLOAT alpha   = atan2(fabs(P_curr_Local.y), fabs(P_curr_Local.x));
    xFLOAT scale   = 2*alpha * PI_inv;
    xFLOAT betaMax;

    if (P_curr_Local.x >= 0 && P_curr_Local.y >= 0)
        betaMax = angleMaxY * scale + angleMaxX * (1.f - scale);
    else if (P_curr_Local.x < 0 && P_curr_Local.y >= 0)
        betaMax = angleMaxY * scale + angleMinX * (1.f - scale);
    else if (P_curr_Local.x >= 0 && P_curr_Local.y < 0)
        betaMax = angleMinY * scale + angleMaxX * (1.f - scale);
    else
        betaMax = angleMinY * scale + angleMinX * (1.f - scale);

    xFLOAT cosMax = cos(betaMax);
    if (N_curr_Local.z >= cosMax) return false;

    return true;
}

void xVConstraintAngular :: CloneTo(xVConstraint *&dst) const
{
    xVConstraintAngular *res = new xVConstraintAngular();
    res->particleRootB = particleRootB;
    res->particleRootE = particleRootE;
    res->particle      = particle;
    res->angleMaxX     = angleMaxX;
    res->angleMaxY     = angleMaxY;
    res->angleMinX     = angleMinX;
    res->angleMinY     = angleMinY;
    res->minZ          = minZ;
    res->maxZ          = maxZ;
    dst = res;
}

void xVConstraintAngular :: Save( FILE *file )
{
    xVConstraint::Save(file);
    fwrite(&particleRootB, sizeof(xWORD), 1, file);
    fwrite(&particleRootE, sizeof(xWORD), 1, file);
    fwrite(&particle, sizeof(xWORD), 1, file);
    fwrite(&angleMaxX, sizeof(xFLOAT), 1, file);
    fwrite(&angleMaxY, sizeof(xFLOAT), 1, file);
    fwrite(&angleMinX, sizeof(xFLOAT), 1, file);
    fwrite(&angleMinY, sizeof(xFLOAT), 1, file);
    fwrite(&minZ, sizeof(xFLOAT), 1, file);
    fwrite(&maxZ, sizeof(xFLOAT), 1, file);
}

xVConstraint * xVConstraintAngular :: Load( FILE *file )
{
    fread(&particleRootB, sizeof(xWORD), 1, file);
    fread(&particleRootE, sizeof(xWORD), 1, file);
    fread(&particle, sizeof(xWORD), 1, file);
    fread(&angleMaxX, sizeof(xFLOAT), 1, file);
    fread(&angleMaxY, sizeof(xFLOAT), 1, file);
    fread(&angleMinX, sizeof(xFLOAT), 1, file);
    fread(&angleMinY, sizeof(xFLOAT), 1, file);
    fread(&minZ, sizeof(xFLOAT), 1, file);
    fread(&maxZ, sizeof(xFLOAT), 1, file);
    return this;
}
