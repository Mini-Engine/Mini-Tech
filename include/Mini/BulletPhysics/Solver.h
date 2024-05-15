#pragma once

#include "Gaffer/StringPlug.h"
#include "Gaffer/ArrayPlug.h"
#include "Gaffer/TypedObjectPlug.h"

#include "GafferScene/SceneProcessor.h"
#include "GafferScene/ScenePlug.h"
#include "GafferScene/FilterPlug.h"

#include "IECoreScene/SceneInterface.h"
#include "IECoreScene/MeshPrimitive.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <GafferScene/ScenePlug.h>
#include <GafferScene/SceneAlgo.h>



namespace MiniGaffer
{
    class Solver
    {
    public:
        static std::shared_ptr<Solver> FindOrCreateSolver(const GafferScene::ScenePlug* scenePlug, float frame);
        static IECore::MurmurHash HashPhysicsInputScene(const GafferScene::ScenePlug* scenePlug);

        ~Solver();

        using MeshHandle = int;
        MeshHandle AddMesh(const GafferScene::ScenePlug::ScenePath& path, IECoreScene::ConstMeshPrimitivePtr mesh, IECore::ConstCompoundObjectPtr attributes, const Imath::M44f& transform);
        [[nodiscard]] MeshHandle FindHandle(const GafferScene::ScenePlug::ScenePath& path) const;

        Imath::M44f GetTransform(MeshHandle handle);
        Imath::V3f GetVelocity(int frame, MeshHandle handle);
        Imath::V3f GetAngularVelocity(int frame, MeshHandle handle);
        void Simulate(float frame);
    protected:
        Solver();
    private:

        btAlignedObjectArray<btCollisionShape*> collisionShapes;
        btDiscreteDynamicsWorld* dynamicsWorld;
        btSequentialImpulseConstraintSolver* solver;
        btBroadphaseInterface* overlappingPairCache;
        btCollisionDispatcher* dispatcher;
        btDefaultCollisionConfiguration* collisionConfiguration;

        using PathMap = std::map<GafferScene::ScenePlug::ScenePath, MeshHandle>;
        PathMap pathToHandle;

        struct ObjectState
        {
            Imath::M44f transform;
        };

        using CachedFrameArray = std::vector<ObjectState>;

        using Cache = std::map<int, CachedFrameArray>;
        Cache cache;

        float timestep = 1.0f / 300.0f;
        int substeps = 1;
    };
}
