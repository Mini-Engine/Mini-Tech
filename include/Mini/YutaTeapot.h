#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectSource.h"

#include "Gaffer/NumericPlug.h"

namespace MiniGaffer
{
    /// Creates a Utah teapot mesh, tessellated from Martin Newell's 32 bicubic Bezier patches.
    ///
    /// The teapot sits on the ground plane with Y up and the spout pointing down +X. At a
    /// `size` of 1 it is 3.15 units tall. Patch seams are welded, so the result is a single
    /// connected mesh with smooth vertex normals and facevarying `uv`s from the patch parameters.
    class YutaTeapot : public GafferScene::ObjectSource
    {
        public :

            explicit YutaTeapot( const std::string &name=defaultName<YutaTeapot>() );
            ~YutaTeapot() override = default;

            Gaffer::FloatPlug *sizePlug();
            const Gaffer::FloatPlug *sizePlug() const;

            Gaffer::IntPlug *divisionsPlug();
            const Gaffer::IntPlug *divisionsPlug() const;

            Gaffer::BoolPlug *lidPlug();
            const Gaffer::BoolPlug *lidPlug() const;

            Gaffer::BoolPlug *bottomPlug();
            const Gaffer::BoolPlug *bottomPlug() const;

            IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::YutaTeapot, MiniGaffer::TypeId::YutaTeapotTypeId, GafferScene::ObjectSource );

            void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

        protected :

            void hashSource( const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
            IECore::ConstObjectPtr computeSource( const Gaffer::Context *context ) const override;

        private :

            static size_t g_firstPlugIndex;

    };

    IE_CORE_DECLAREPTR( YutaTeapot )

} // namespace MiniGaffer
