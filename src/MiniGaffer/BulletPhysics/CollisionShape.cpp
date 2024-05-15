#include <Mini/BulletPhysics/CollisionShape.h>
#include <Mini/BulletPhysics/Convert.h>

namespace MiniGaffer
{
    btCollisionShape* MakeShape(const IECoreScene::ConstMeshPrimitivePtr& mesh, IECore::ConstCompoundObjectPtr attributes)
    {
        // todo: make a cache of collision shapes
        // todo: often we'll have lots of the same object instanced to we should only construct the collision shape once
        // todo: for each unique shape / physics attribute set.

        ParameterReader reader(attributes);
        const auto collisionShape = reader.Get<std::string>("physics::collision_shape", "box");

        if (collisionShape == "convex_hull")
        {
            auto itP = mesh->variables.find("P");
            if (itP != mesh->variables.end())
            {
                auto positionData = IECore::runTimeCast<IECore::V3fVectorData>(itP->second.data);
                const auto& readable = positionData->readable();

                auto hullShape = new btConvexHullShape ((float*) readable.data(), readable.size(), sizeof(Imath::V3f));
                return hullShape;
            }
        }
        else if (collisionShape == "triangle_mesh")
        {
            // needs interface to read meshes
            //auto shape = new btBvhTriangleMeshShape()
        }
        else if (collisionShape == "bounding_box")
        {
            return new btBoxShape(btVector3(mesh->bound().size().x/2.0f,mesh->bound().size().y/2.0f,mesh->bound().size().z /2.0f));
        }
        else if (collisionShape == "sphere")
        {
            const float maxDimension = std::max(std::max(mesh->bound().size().x,mesh->bound().size().y), mesh->bound().size().z );
            return new btSphereShape(maxDimension / 2.0f);
        }

        return new btBoxShape(btVector3(mesh->bound().size().x/2.0f,mesh->bound().size().y/2.0f,mesh->bound().size().z /2.0f));

    }
}