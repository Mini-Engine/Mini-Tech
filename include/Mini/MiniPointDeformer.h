#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"


#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

namespace Gaffer
{
    IE_CORE_FORWARDDECLARE( StringPlug )
} // namespace Gaffer

namespace MiniGaffer
{
    /// Deforms Geometry based on points and bindings
    class MiniPointDeformer : public GafferScene::ObjectProcessor
    {
        public :

            explicit MiniPointDeformer(const std::string &name=defaultName<MiniPointDeformer>() );
            ~MiniPointDeformer() override = default;

            Gaffer::StringPlug *CageLocationPlug();
            const Gaffer::StringPlug *CageLocationPlug() const;

            Gaffer::StringPlug *WeightsPrimVarPlug();
            const Gaffer::StringPlug *WeightsPrimVarPlug() const;

            Gaffer::StringPlug *IndicesPrimVarPlug();
            const Gaffer::StringPlug *IndicesPrimVarPlug() const;

            Gaffer::StringPlug *BindTransformPlug();
            const Gaffer::StringPlug *BindTransformPlug() const;

            IE_CORE_DECLARERUNTIMETYPEDEXTENSION(MiniGaffer::MiniPointDeformer, MiniGaffer::TypeId::MiniPointDeformTypeId, ObjectProcessor );

        protected :

            bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
            void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
            IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
            Gaffer::ValuePlug::CachePolicy processedObjectComputeCachePolicy() const override;

        private :

            static size_t g_firstPlugIndex;

    };

    IE_CORE_DECLAREPTR( MiniPointDeformer )

    } // namespace MiniGaffer
