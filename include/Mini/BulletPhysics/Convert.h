#pragma once

#include <IECore/CompoundObject.h>
#include <IECore/TypedData.h>


#include <Imath/ImathMatrix.h>
#include <Imath/ImathVec.h>
#include <Imath/ImathQuat.h>

#include <bullet/btBulletDynamicsCommon.h>
#include <IECore/SimpleTypedData.h>

namespace MiniGaffer
{
    btTransform Convert(const Imath::M44f& transform);
    Imath::M44f Convert(const btTransform& transform);

    // Read physics attributes from Cortex CompoundObject
    struct ParameterReader
    {
        explicit ParameterReader(IECore::ConstCompoundObjectPtr attributes)
        : m_attributes(attributes) {}

        template<typename T>
        bool Get(T& parameter, const std::string& attributeName, const T defaultValue = T{})
        {
            if (auto typedData = m_attributes->member<IECore::TypedData<T> >(attributeName))
            {
                parameter = typedData->readable();
                return true;
            }
            parameter = defaultValue;
            return false;
        }

        template<typename T>
        T Get(const std::string& attributeName, const T defaultValue)
        {
            if (auto typedData = m_attributes->member<IECore::TypedData<T> >(attributeName))
            {
                return typedData->readable();
            }

            return defaultValue;
        }

        IECore::ConstCompoundObjectPtr m_attributes;

    };

    // todo implement btStridingMeshInterface to a IECoreScene::Mesh
}