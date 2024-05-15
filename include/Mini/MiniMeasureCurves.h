#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/SceneInterface.h"

namespace MiniGaffer
{
    class MiniMeasureCurves : public GafferScene::ObjectProcessor
    {
    public:

        explicit MiniMeasureCurves( const std::string &name = defaultName<MiniMeasureCurves>() );
        ~MiniMeasureCurves() = default;

        Gaffer::StringPlug *lengthNamePlug();
        const Gaffer::StringPlug *lengthNamePlug() const;

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniMeasureCurves, TypeId::MiniMeasureCurvesTypeId, GafferScene::FilteredSceneProcessor );

    protected:
        bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
        void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
        Gaffer::ValuePlug::CachePolicy processedObjectComputeCachePolicy() const override;

    private:
        static size_t g_firstPlugIndex;
    };
    
} // namespace MiniGaffer