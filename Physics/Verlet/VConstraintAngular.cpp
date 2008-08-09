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
    xVector3 &P_rootB = system->P_current[particleRootB];
    xVector3 &P_rootE = system->P_current[particleRootE];
    xVector3 &P_curr  = system->P_current[particle];

    xQuaternion QT_parent, QT_current;
    // OPTIMIZE: calc only needed quats
    system->spine->CalcQuats(system->P_current, system->QT_boneSkew,
                             0, system->MX_WorldToModel_T, xVector3::Create(0.f,0.f,0.f));
    xBoneCalculateQuatForVerlet(*system->spine, particle, QT_parent, QT_current);
    QT_current = xQuaternion::product(QT_parent, QT_current);

    xBone   &bone    = system->spine->L_bones[particle];
    xVector3 N_up    = system->MX_ModelToWorld.preTransformV(xQuaternion::rotate(QT_parent, bone.P_end-bone.P_begin)).normalize();
    xVector3 N_front = system->MX_ModelToWorld.preTransformV(xQuaternion::rotate(QT_current, bone.getFront()));

    xMatrix  MX_WorldToBone = xMatrixFromVectors( N_front, N_up, - P_rootE ).invert();
    xVector3 P_curr_Local   = MX_WorldToBone.preTransformP( P_curr );
    xVector3 N_curr_Local   = xVector3::Normalize( P_curr_Local );

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
    int I_gamma = (P_curr_Local.x < 0) + 2 * (P_curr_Local.y < 0);

    xFLOAT cosMax = cos(L_gammaMax[I_gamma]);
    if (N_curr_Local.z >= cosMax) return false;

    xFLOAT S_minFix_Sqr = xFLOAT_HUGE_POSITIVE;
    xQuaternion QT_minFix; //QT_minFix.zeroQ();

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
            QT_minFix = xQuaternion::getRotation(N_curr_Local, P_new_Local);
        }
    }

    xFLOAT w1 = system->M_weight_Inv[particle];
    xFLOAT w2 = system->M_weight_Inv[particleRootE];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    xQuaternion QT_curr = xQuaternion::interpolate(QT_minFix, w1);
    xQuaternion QT_root = xQuaternion::interpolate(QT_minFix, w2);
    xMatrix &MX_BoneToWorld = MX_WorldToBone.invert();
    P_rootE = MX_BoneToWorld.preTransformP( xQuaternion::rotate(QT_root, -P_curr_Local) + P_curr_Local );
    P_curr  = MX_BoneToWorld.preTransformP( xQuaternion::rotate(QT_curr, P_curr_Local) );

    //system->P_previous[particleRootE] = P_rootE;
    //system->P_previous[particle] = P_curr;

    return true;
}
bool VConstraintAngular :: Test(const xVector3 &P_rootB, const xVector3 &P_rootE, const xVector3 &P_curr,
                                 const xVector3 &N_up, const xVector3 &N_front) const
{
    xMatrix MX_WorldToBone = xMatrixFromVectors(N_front, N_up, -P_rootE).invert();
    xVector3 P_curr_Local  = MX_WorldToBone.preTransformP( P_curr );
    xVector3 N_curr_Local  = xVector3::Normalize( P_curr_Local );

    xFLOAT r_Inv = 1.f / sqrt(N_curr_Local.x*N_curr_Local.x+N_curr_Local.y*N_curr_Local.y);
    xFLOAT cosAlpha = fabs(N_curr_Local.x)*r_Inv,
           sinAlpha = fabs(N_curr_Local.y)*r_Inv,
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
