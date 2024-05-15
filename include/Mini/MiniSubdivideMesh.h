#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/SceneInterface.h"

namespace MiniGaffer
{
    class MiniSubdivideMesh : public GafferScene::ObjectProcessor
    {
    public:

        explicit MiniSubdivideMesh( const std::string &name = defaultName<MiniSubdivideMesh>() );
        ~MiniSubdivideMesh() = default;

        Gaffer::IntPlug *levelPlug();
        const Gaffer::IntPlug *levelPlug() const;

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniSubdivideMesh, TypeId::MiniSubdivideMeshTypeId, GafferScene::FilteredSceneProcessor );
    protected:
        bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
        void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
        Gaffer::ValuePlug::CachePolicy processedObjectComputeCachePolicy() const override;
    private:
        static size_t g_firstPlugIndex;
    };
} // namespace MiniGaffer