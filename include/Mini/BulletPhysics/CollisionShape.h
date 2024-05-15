#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <IECoreScene/MeshPrimitive.h>
#include <IECore/CompoundObject.h>
namespace MiniGaffer
{
    btCollisionShape* MakeShape(const IECoreScene::ConstMeshPrimitivePtr& mesh, IECore::ConstCompoundObjectPtr attributes);
}