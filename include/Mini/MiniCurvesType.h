#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/SceneInterface.h"

namespace MiniGaffer
{
    class MiniCurvesType : public GafferScene::ObjectProcessor
    {
    public:

        explicit MiniCurvesType( const std::string &name = defaultName<MiniCurvesType>() );
        ~MiniCurvesType() = default;

        Gaffer::IntPlug *basisPlug();
        const Gaffer::IntPlug *basisPlug() const;

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniCurvesType, TypeId::MiniCurvesTypeId, GafferScene::FilteredSceneProcessor );

    protected:
        bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
        void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
        Gaffer::ValuePlug::CachePolicy processedObjectComputeCachePolicy() const override;

    private:
        static size_t g_firstPlugIndex;
    };

} // namespace MiniGaffer