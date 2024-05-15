#pragma once

#include "TypeIds.h"

#include "Gaffer/StringPlug.h"
#include "Gaffer/ArrayPlug.h"
#include "Gaffer/TypedObjectPlug.h"

#include "GafferScene/SceneProcessor.h"
#include "GafferScene/ScenePlug.h"
#include "GafferScene/FilterPlug.h"

#include "IECoreScene/SceneInterface.h"
#include "IECoreScene/MeshPrimitive.h"

#include <Mini/BulletPhysics/Solver.h>

namespace MiniGaffer
{

    class MiniBullet : public GafferScene::SceneProcessor
    {
    public:
        explicit MiniBullet ( const std::string &name = defaultName<MiniBullet>());
        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniBullet, TypeId::MiniBulletTypeId, GafferScene::SceneProcessor );

        Gaffer::FloatPlug *framePlug();
        const Gaffer::FloatPlug *framePlug() const;

        void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;
    protected:

        void hashTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;
        Imath::M44f computeTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const override;

    private :

        static size_t g_firstPlugIndex;


    };

    IE_CORE_DECLAREPTR(MiniBullet)
} // MiniGaffer