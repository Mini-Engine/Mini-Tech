#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/SceneInterface.h"

namespace MiniGaffer
{
    class MiniGrowCurves : public GafferScene::ObjectProcessor
    {
    public:

        explicit MiniGrowCurves( const std::string &name = defaultName<MiniGrowCurves>() );
        ~MiniGrowCurves() = default;

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniGrowCurves, TypeId::MiniGrowCurvesTypeId, GafferScene::FilteredSceneProcessor );

    protected:
        bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
        void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
        Gaffer::ValuePlug::CachePolicy processedObjectComputeCachePolicy() const override;

    private:
        static size_t g_firstPlugIndex;
    };

} // namespace MiniGaffer