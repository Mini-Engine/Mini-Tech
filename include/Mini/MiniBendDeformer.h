#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/TransformPlug.h"

namespace MiniGaffer
{
    /// Bends the points of the filtered geometry like Maya's Bend nonlinear deformer.
    ///
    /// In the space of the `handle` transform the geometry is bent along the handle's Y axis,
    /// towards its X axis, by `curvature` ( radians per unit length ). Only the part of the
    /// geometry between `lowBound` and `highBound` along Y is curved. Geometry beyond the
    /// bounds continues in a straight line from the end of the curve.
    class MiniBendDeformer : public GafferScene::ObjectProcessor
    {
        public :

            explicit MiniBendDeformer( const std::string &name=defaultName<MiniBendDeformer>() );
            ~MiniBendDeformer() override = default;

            Gaffer::FloatPlug *envelopePlug();
            const Gaffer::FloatPlug *envelopePlug() const;

            Gaffer::FloatPlug *curvaturePlug();
            const Gaffer::FloatPlug *curvaturePlug() const;

            Gaffer::FloatPlug *lowBoundPlug();
            const Gaffer::FloatPlug *lowBoundPlug() const;

            Gaffer::FloatPlug *highBoundPlug();
            const Gaffer::FloatPlug *highBoundPlug() const;

            Gaffer::TransformPlug *handlePlug();
            const Gaffer::TransformPlug *handlePlug() const;

            IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniBendDeformer, MiniGaffer::TypeId::MiniBendDeformerTypeId, GafferScene::ObjectProcessor );

        protected :

            bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
            void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
            IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;

        private :

            static size_t g_firstPlugIndex;

    };

    IE_CORE_DECLAREPTR( MiniBendDeformer )

} // namespace MiniGaffer
