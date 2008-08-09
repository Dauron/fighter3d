#include "FigureCollider.h"

#include "../../Math/Figures/xSphere.h"
#include "../../Math/Figures/xCapsule.h"
#include "../../Math/Figures/xBoxO.h"
#include "../../Math/Figures/xCylinder.h"
#include "../../Math/Figures/xMesh.h"

using namespace ::Math::Figures;
using namespace ::Physics::Colliders;

#include "AxisSpans.h"

#include "SphereSphere.h"
#include "SphereCapsule.h"
#include "SphereBoxO.h"
//#include "SphereCylinder.h"
#include "SphereMesh.h"
#include "CapsuleCapsule.h"
#include "CapsuleBoxO.h"
//#include "CapsuleCylinder.h"
//#include "CapsuleMesh.h"
#include "BoxOBoxO.h"
//#include "BoxOCylinder.h"
//#include "BoxMesh.h"
//#include "CylinderCylinder.h"
//#include "CylinderMesh.h"

////////////////////////////// FigureCollider :: Test

bool          FigureCollider :: Test    (const xIFigure3d *figure1, const xIFigure3d *figure2)
{
    // Sphere
    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Sphere)
        return TestSphereSphere ( *((xSphere*) figure1), *((xSphere*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Capsule)
        return TestSphereCapsule ( *((xSphere*) figure1), *((xCapsule*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::BoxOriented)
        return TestSphereBoxO ( *((xSphere*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Mesh)
        return TestSphereMesh( *((xSphere*) figure1), *((xMesh*) figure2) );

    // Capsule
    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Sphere)
        return TestSphereCapsule ( *((xSphere*) figure2), *((xCapsule*) figure1) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Capsule)
        return TestCapsuleCapsule ( *((xCapsule*) figure1), *((xCapsule*) figure2) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::BoxOriented)
        return TestCapsuleBoxO ( *((xCapsule*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Mesh)
        return false;

    // Box Oriented
    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Sphere)
        return TestSphereBoxO ( *((xSphere*) figure2), *((xBoxO*) figure1) );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Capsule)
        return TestCapsuleBoxO ( *((xCapsule*) figure2), *((xBoxO*) figure1) );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::BoxOriented)
        return TestBoxOBoxO ( *((xBoxO*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Mesh)
        return false;

    // Cylinder
    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Sphere)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Capsule)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::BoxOriented)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Mesh)
        return false;

    // Mesh
    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Sphere)
        return TestSphereMesh( *((xSphere*) figure2), *((xMesh*) figure1) );

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Capsule)
        return false;

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::BoxOriented)
        return false;

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Mesh)
        return false;

    return false;
}

////////////////////////////// FigureCollider :: Collide

CollisionInfo FigureCollider :: Collide (const xIFigure3d *figure1, const xIFigure3d *figure2)
{
    // Sphere
    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Sphere)
        return CollideSphereSphere ( *((xSphere*) figure1), *((xSphere*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Capsule)
        return CollideSphereCapsule ( *((xSphere*) figure1), *((xCapsule*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::BoxOriented)
        return CollideSphereBoxO ( *((xSphere*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Cylinder)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Mesh)
        return CollideSphereMesh ( *((xSphere*) figure1), *((xMesh*) figure2) );

    // Capsule
    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Sphere)
        return CollideSphereCapsule ( *((xSphere*) figure2), *((xCapsule*) figure1) ).invert();

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Capsule)
        return CollideCapsuleCapsule ( *((xCapsule*) figure1), *((xCapsule*) figure2) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::BoxOriented)
        return CollideCapsuleBoxO ( *((xCapsule*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Cylinder)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Mesh)
        return CollisionInfo(false);

    // Box Oriented
    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Sphere)
        return CollideSphereBoxO ( *((xSphere*) figure2), *((xBoxO*) figure1) ).invert();

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Capsule)
        return CollideCapsuleBoxO ( *((xCapsule*) figure2), *((xBoxO*) figure1) ).invert();

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::BoxOriented)
        return CollideBoxOBoxO ( *((xBoxO*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Cylinder)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Mesh)
        return CollisionInfo(false);

    // Cylinder
    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Sphere)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Capsule)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::BoxOriented)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Cylinder)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Mesh)
        return CollisionInfo(false);

    // Mesh
    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Sphere)
        return CollideSphereMesh ( *((xSphere*) figure2), *((xMesh*) figure1) ).invert();

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Capsule)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::BoxOriented)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Cylinder)
        return CollisionInfo(false);

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Mesh)
        return CollisionInfo(false);

    return CollisionInfo(false);
}
