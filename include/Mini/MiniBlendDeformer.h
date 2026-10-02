#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

#include "Gaffer/NumericPlug.h"

namespace Gaffer
{
    IE_CORE_FORWARDDECLARE( StringPlug )
    IE_CORE_FORWARDDECLARE( ArrayPlug )
} // namespace Gaffer

namespace MiniGaffer
{
    /// Blends the points of the filtered geometry towards one or more target shapes, like Maya's blendShape.
    ///
    /// Each target contributes `weight * ( targetP - baseP )` to the points, where `baseP` is the
    /// points of the optional base location or, if none is given, the points being deformed.
    /// The sum is scaled by the envelope and an optional per vertex mask primitive variable.
    class MiniBlendDeformer : public GafferScene::ObjectProcessor
    {
        public :

            explicit MiniBlendDeformer( const std::string &name=defaultName<MiniBlendDeformer>() );
            ~MiniBlendDeformer() override = default;

            Gaffer::FloatPlug *envelopePlug();
            const Gaffer::FloatPlug *envelopePlug() const;

            Gaffer::StringPlug *baseLocationPlug();
            const Gaffer::StringPlug *baseLocationPlug() const;

            Gaffer::StringPlug *maskPrimVarPlug();
            const Gaffer::StringPlug *maskPrimVarPlug() const;

            /// Array of targets, each with `enabled`, `location` and `weight` children.
            Gaffer::ArrayPlug *targetsPlug();
            const Gaffer::ArrayPlug *targetsPlug() const;

            IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniBlendDeformer, MiniGaffer::TypeId::MiniBlendDeformerTypeId, GafferScene::ObjectProcessor );

        protected :

            bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
            void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
            IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
            Gaffer::ValuePlug::CachePolicy processedObjectComputeCachePolicy() const override;

        private :

            static size_t g_firstPlugIndex;

    };

    IE_CORE_DECLAREPTR( MiniBlendDeformer )

} // namespace MiniGaffer
