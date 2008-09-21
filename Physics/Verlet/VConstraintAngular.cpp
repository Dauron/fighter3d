#include "VConstraintAngular.h"
#include "VerletSolver.h"
#include "../../Models/lib3dx/xSkeleton.h"

xFLOAT FindGammaMax(xFLOAT a, xFLOAT b, xFLOAT tanAlpha, xFLOAT cosAlpha)
{
    if (cosAlpha > EPSILON)
    {
        if (a > EPSILON && b > EPSILON)
        {
            xFLOAT beta = atan ( tanAlpha * a / b );
            xFLOAT cosBeta2 = cos(beta); cosBeta2 *= cosBeta2;
            xFLOAT sinBeta2 = 1 - cosBeta2;
            return sqrt( a*a*cosBeta2 + b*b*sinBeta2 );
        }
        return 0.f;
    }
    return a > EPSILON ? b : 0.f;
}

bool VConstraintAngular :: Satisfy(VerletSystem *system)
{
    xVector3 &P_rootE = system->P_current[particleRootE];
    xVector3 &P_curr  = system->P_current[particle];

    xQuaternion QT_parent, QT_current, QT_bone;

    // calc only needed quats
    bool FL_calculate[100]; FL_calculate[0] = true;
    for (int i = 1; i < system->I_particles; ++i) FL_calculate[i] = false;
    xBYTE idx = particle;
    while (!FL_calculate[idx]) { FL_calculate[idx] = true; idx = system->Spine->L_bones[idx].ID_parent; }
    idx = particleRootE ? particleRootE : particleRootB;
    while (!FL_calculate[idx]) { FL_calculate[idx] = true; idx = system->Spine->L_bones[idx].ID_parent; }
    system->Spine->CalcQuats(system->P_current, system->QT_boneSkew, FL_calculate,
                             0, system->MX_WorldToModel_T);
    xBoneCalculateQuatForVerlet(*system->Spine, particle, QT_parent, QT_bone);
    QT_current = xQuaternion::Product(QT_parent, QT_bone);

    xBone &bone = system->Spine->L_bones[particle];

    xVector3 N_up_loc   = (bone.P_end-bone.P_begin).normalize();
    //xPoint3  N_up_rot   = QT_bone.rotate(N_up_loc);
    //xFLOAT   sinSkew = xVector3::DotProduct(N_up_rot, QT_bone.vector3);
    //xFLOAT   cosSkew = sqrtf(1.f - sinSkew*sinSkew);
    //xQuaternion QT_skew; QT_skew.init(N_up_rot*sinSkew, cosSkew);

    xVector3 N_up = system->MX_ModelToWorld.preTransformV(QT_parent.rotate(N_up_loc));
    xVector3 N_front = system->MX_ModelToWorld.preTransformV(QT_current.rotate(bone.getFront()));

    xMatrix  MX_WorldToBone = xMatrixFromVectors( N_front, N_up, - P_rootE ).invert();
    xVector3 P_curr_Local   = MX_WorldToBone.preTransformP( P_curr );
    xVector3 N_curr_Local   = xVector3::Normalize( P_curr_Local );

    if (N_curr_Local.z > 0.99f /*&& QT_skew.w > 0.92f*/) return false;

    xFLOAT w_max = max(system->W_boneMix[particle], system->W_boneMix[particleRootE]);
    xFLOAT w_min = min(system->W_boneMix[particle], system->W_boneMix[particleRootE]);
    xFLOAT w_mean = w_min + 0.8 * (w_max-w_min);
    system->W_boneMix[particle] = w_mean;
    system->W_boneMix[particleRootE] = w_mean;

    xFLOAT w1 = system->M_weight_Inv[particle];
    xFLOAT w2 = system->M_weight_Inv[particleRootE];
    if (system->FL_attached[particle])      w1 *= 0.1f;
    if (system->FL_attached[particleRootE]) w2 *= 0.1f;
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;
/*
    sinSkew = 0.f;
    xQuaternion QT_unSkew;
    if (QT_skew.w <= 0.92f)
    {
        xFLOAT angleSkew = asin(sinSkew) - PI * 0.125f;
        cosSkew          = cos(angleSkew);
        sinSkew          = sin(angleSkew);
        QT_unSkew.init(0,0,sinSkew,cosSkew);
        N_curr_Local = QT_unSkew.rotate(N_curr_Local);
        QT_unSkew.vector3 = N_up_rot * -sinSkew;
    }
    else
        QT_unSkew.zeroQ();
*/
    xFLOAT r_Inv = 1.f / sqrt(N_curr_Local.x*N_curr_Local.x+N_curr_Local.y*N_curr_Local.y);
    xFLOAT cosAlpha = fabs(N_curr_Local.x)*r_Inv,
           sinAlpha = fabs(N_curr_Local.y)*r_Inv,
           tanAlpha;
    if (cosAlpha > EPSILON)
        tanAlpha = N_curr_Local.y / N_curr_Local.x;
    else
        tanAlpha = 0.f; // infinity, but don't care

    xFLOAT L_gammaMax[4];
    L_gammaMax[0] = FindGammaMax(angleMaxX, angleMaxY, tanAlpha, cosAlpha);
    L_gammaMax[1] = FindGammaMax(angleMinX, angleMaxY, tanAlpha, cosAlpha);
    L_gammaMax[2] = FindGammaMax(angleMaxX, angleMinY, tanAlpha, cosAlpha);
    L_gammaMax[3] = FindGammaMax(angleMinX, angleMinY, tanAlpha, cosAlpha);
    // MaxMax = 0, MinMax = 1, MaxMin = 2, MinMin = 3
    int I_gamma = (N_curr_Local.x < 0) + 2 * (N_curr_Local.y < 0);

    xFLOAT cosMax = cos(L_gammaMax[I_gamma]);
    if (N_curr_Local.z >= cosMax /*&& QT_skew.w > 0.92f*/) return false;

    xFLOAT S_minFix_Sqr = xFLOAT_HUGE_POSITIVE;
    xQuaternion QT_minFix;

    if (N_curr_Local.z < cosMax)
    {
        for (int I_gamma = 0; I_gamma < 4; ++I_gamma)
        {
            // {0,1,2,3} => {0,1} => {0,2} => {-1,1} => {1,-1}
            int xSign = - ((I_gamma % 2) * 2 - 1) ;
            // {0,1,2,3} => {1,0} => {2,0} => {1,-1}
            int ySign = (I_gamma < 2) * 2 - 1;

            xVector3 P_new_Local;
            xFLOAT xy     = sin(L_gammaMax[I_gamma]-DegToRad(1.f));
            P_new_Local.x = xSign * xy * cosAlpha;
            P_new_Local.y = ySign * xy * sinAlpha;
            P_new_Local.z = cos(L_gammaMax[I_gamma]-DegToRad(1.f));

            xFLOAT S_curFix_Sqr = (N_curr_Local - P_new_Local).lengthSqr();
            if (S_curFix_Sqr < S_minFix_Sqr)
            {
                S_minFix_Sqr = S_curFix_Sqr;
                QT_minFix = xQuaternion::GetRotation(N_curr_Local, P_new_Local);
            }
        }
    }
    else
        QT_minFix.zeroQ();

    xQuaternion QT_curr = xQuaternion::InterpolateFull(QT_minFix, w1);
    xQuaternion QT_root = xQuaternion::InterpolateFull(QT_minFix, w2);
    xMatrix &MX_BoneToWorld = MX_WorldToBone.invert();
    P_rootE = MX_BoneToWorld.preTransformP( QT_root.rotate(-P_curr_Local) + P_curr_Local );
    P_curr  = MX_BoneToWorld.preTransformP( QT_curr.rotate( /*QT_unSkew.rotate*/( P_curr_Local )) );

    system->QT_boneSkew[particle] = xQuaternion::Product(system->QT_boneSkew[particle], /*xQuaternion::Product(*/QT_curr/*, QT_unSkew)*/);
    /*QT_unSkew.vector3.invert();
    for (int i = 0; i < bone.I_kids; ++i)
    {
        int id = bone.ID_kids[i];
        system->QT_boneSkew[id] = xQuaternion::Product(system->QT_boneSkew[id], QT_unSkew);
    }*/

    if (particleRootE)
    {
        QT_root.vector3.invert();
        system->QT_boneSkew[particleRootE] = xQuaternion::Product(system->QT_boneSkew[particleRootE], QT_root);
    }
    else
    {
        QT_root.vector3.invert();
        system->QT_boneSkew[particleRootB] = xQuaternion::Product(system->QT_boneSkew[particleRootB], QT_root);
    }

    //system->P_previous[particleRootE] = P_rootE;
    //system->P_previous[particle] = P_curr;

    return true;
}
bool VConstraintAngular :: Test(const xVector3 &P_rootB, const xVector3 &P_rootE, const xVector3 &P_curr,
                                const xVector3 &N_up, const xVector3 &N_front, const xQuaternion &QT_bone) const
{
    xMatrix MX_WorldToBone = xMatrixFromVectors(N_front, N_up, -P_rootE).invert();
    xVector3 P_curr_Local  = MX_WorldToBone.preTransformP( P_curr );
    xVector3 N_curr_Local  = xVector3::Normalize( P_curr_Local );

    //xPoint3 P_rot  = QT_bone.rotate(N_up);
    //xQuaternion QT_rot = xQuaternion::GetRotation(P_rot, N_up);
    //xFLOAT cosa = xQuaternion::Product(QT_bone, QT_rot).w;
    //if (cosa < 0.92f)
    //    return true;

    xFLOAT r_Inv = 1.f / sqrt(N_curr_Local.x*N_curr_Local.x+N_curr_Local.y*N_curr_Local.y);
    xFLOAT cosAlpha = fabs(N_curr_Local.x)*r_Inv,
           tanAlpha;
    if (cosAlpha > EPSILON)
        tanAlpha = N_curr_Local.y / N_curr_Local.x;
    else
        tanAlpha = 0.f; // infinity, but don't care
    xFLOAT gammaMax;

    if (P_curr_Local.x >= 0 && P_curr_Local.y >= 0)
        gammaMax = FindGammaMax(angleMaxX, angleMaxY, tanAlpha, cosAlpha);
    else if (P_curr_Local.x < 0 && P_curr_Local.y >= 0)
        gammaMax = FindGammaMax(angleMinX, angleMaxY, tanAlpha, cosAlpha);
    else if (P_curr_Local.x >= 0 && P_curr_Local.y < 0)
        gammaMax = FindGammaMax(angleMaxX, angleMinY, tanAlpha, cosAlpha);
    else
        gammaMax = FindGammaMax(angleMinX, angleMinY, tanAlpha, cosAlpha);

    xFLOAT cosMax = cos(gammaMax);
    if (N_curr_Local.z >= cosMax) return false;

    return true;
}

void VConstraintAngular :: CloneTo(VConstraint *&dst) const
{
    VConstraintAngular *res = new VConstraintAngular();
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

void VConstraintAngular :: Save( FILE *file )
{
    VConstraint::Save(file);
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

VConstraint * VConstraintAngular :: Load( FILE *file )
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
