#include <Mini/BulletPhysics/Solver.h>
#include <Mini/BulletPhysics/Convert.h>
#include <Mini/BulletPhysics/CollisionShape.h>

namespace
{
    std::mutex gCreateSolverMutex;
}
namespace MiniGaffer
{
    IECore::MurmurHash Solver::HashPhysicsInputScene(const GafferScene::ScenePlug* scenePlug)
    {
        IECore::MurmurHash h;

        auto matcher = GafferScene::SceneAlgo::findAll( scenePlug, [](const GafferScene::ScenePlug* , const GafferScene::ScenePlug::ScenePath&) { return true;} );
        for(IECore::PathMatcher::Iterator it = matcher.begin(); it != matcher.end(); ++it)
        {
            if ( IECoreScene::ConstMeshPrimitivePtr meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>(scenePlug->object(*it)) )
            {
                meshPrimitive->hash(h);
                h.append(scenePlug->attributesHash(*it));
                h.append(scenePlug->transformHash(*it));
            }
        }

        return h;
    }

    std::shared_ptr<Solver> Solver::FindOrCreateSolver(const GafferScene::ScenePlug* scenePlug, float frame)
    {
        static std::map<IECore::MurmurHash, std::shared_ptr<Solver>> solvers;

        auto hash = HashPhysicsInputScene( scenePlug );

        // todo: same solver can be used if we change frames instead of constructing a new solver for different
        // todo: times on the same input state
        hash.append(frame);

        {
            std::lock_guard<std::mutex> guard(gCreateSolverMutex);
            auto it = solvers.find(hash);
            if (it != solvers.end())
            {
                return it->second;
            }
        }

        auto newSolver = std::shared_ptr<Solver> { new Solver };

        auto matcher = GafferScene::SceneAlgo::findAll( scenePlug, [](const GafferScene::ScenePlug* , const GafferScene::ScenePlug::ScenePath&) { return true;} );
        for(IECore::PathMatcher::Iterator it = matcher.begin(); it != matcher.end(); ++it)
        {
            if ( IECoreScene::ConstMeshPrimitivePtr meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>(scenePlug->object(*it)) )
            {
                newSolver->AddMesh(*it, meshPrimitive, scenePlug->attributes(*it), scenePlug->transform(*it));
            }
        }
        newSolver->Simulate(frame);

        {
            std::lock_guard<std::mutex> guard(gCreateSolverMutex);
            solvers[hash] = newSolver;
        }


        return newSolver;
    }

    Solver::Solver()
    {
        ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
        collisionConfiguration = new btDefaultCollisionConfiguration();

        ///use the default collision dispatcher. For parallel processing you can use a different dispatcher (see Extras/BulletMultiThreaded)
        dispatcher = new btCollisionDispatcher(collisionConfiguration);

        ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
        overlappingPairCache = new btDbvtBroadphase();

        ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        solver = new btSequentialImpulseConstraintSolver;

        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
        dynamicsWorld->setGravity(btVector3(0, -10, 0));
    }

    Solver::~Solver()
    {
        //remove the rigid bodies from the dynamics world and delete them
        for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            dynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }

        //delete collision shapes
        for (int j = 0; j < collisionShapes.size(); j++)
        {
            btCollisionShape* shape = collisionShapes[j];
            collisionShapes[j] = 0;
            delete shape;
        }

        //delete dynamics world
        delete dynamicsWorld;

        //delete solver
        delete solver;

        //delete broadphase
        delete overlappingPairCache;

        //delete dispatcher
        delete dispatcher;

        delete collisionConfiguration;
    }


    Solver::MeshHandle Solver::AddMesh(const GafferScene::ScenePlug::ScenePath& path,
                                       IECoreScene::ConstMeshPrimitivePtr mesh,
                                       IECore::ConstCompoundObjectPtr attributes,
                                       const Imath::M44f& transform)
    {
        ParameterReader parameterReader(attributes);

        // Solver Cache
        // Cached simulation state

        // todo build collision from MeshPrimitive
        // todo read attributes from scene
        // todo initial velocity from attribute or previous frame
        // todo visualize collision
        // todo filter for which objects are in the physics scene.

        // todo simple MVP physics, fluid and particle set of nodes
        // todo hair dynamics? destruction? secondary soft body physics?
        // todo smoke or dust simulation?
        // todo magic particles moving though field. OSL to drive, color, size, lifetime,
        // todo bullet / embree for collisions
        // todo should physics work on instancer type objects (points with rotations)

        // todo constraints - constraint objects separate from meshes?
        // todo USD physics integration?

        btCollisionShape* colShape = MakeShape(mesh, attributes);

        collisionShapes.push_back(colShape);

        /// Create Dynamic Objects
        btScalar mass = parameterReader.Get("physics:mass", 1.0f);

        // rigid body is dynamic if and only if mass is non-zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            colShape->calculateLocalInertia(mass, localInertia);

        btTransform startTransform = Convert(transform);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        auto* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

        parameterReader.Get(rbInfo.m_restitution, "physics:restitution");
        parameterReader.Get(rbInfo.m_friction, "physics:friction");
        parameterReader.Get(rbInfo.m_linearDamping, "physics:linear_damping");
        parameterReader.Get(rbInfo.m_angularDamping, "physics:angular_damping");

        auto* body = new btRigidBody(rbInfo);

        dynamicsWorld->addRigidBody(body);

        auto newHandle = static_cast<int>(collisionShapes.size() - 1);
        pathToHandle[path] = newHandle;
        return newHandle;
    }

    Solver::MeshHandle Solver::FindHandle(const GafferScene::ScenePlug::ScenePath& path) const
    {
        const auto it = pathToHandle.find(path);
        if (it != pathToHandle.end())
        {
            return it->second;
        }
        return -1;
    }

    Imath::M44f Solver::GetTransform(MeshHandle handle)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[handle];
        btRigidBody* body = btRigidBody::upcast(obj);
        btTransform trans;

        if (body && body->getMotionState())
        {
            body->getMotionState()->getWorldTransform(trans);
        }
        else
        {
            trans = obj->getWorldTransform();
        }

        Imath::M44f result = Convert(trans);
        return result;
    }

    void Solver::Simulate(float frame)
    {
        float fps = 24.0f;
        int steps = static_cast<int>(frame / (fps * timestep));
        // todo cache simulation

        for (int i = 0; i < steps; i++)
        {
            dynamicsWorld->stepSimulation(timestep, substeps);
        }
    }
}

