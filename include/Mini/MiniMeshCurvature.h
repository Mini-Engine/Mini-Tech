#pragma once

#include "TypeIds.h"

#include "GafferScene/ObjectProcessor.h"

namespace Gaffer
{

IE_CORE_FORWARDDECLARE( StringPlug )

} // namespace Gaffer

namespace MiniGaffer
{

/// Computes curvature of a mesh primitive.
class MiniMeshCurvature : public GafferScene::ObjectProcessor
{

	public :

		MiniMeshCurvature(const std::string &name=defaultName<MiniMeshCurvature>() );
		~MiniMeshCurvature() override;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION(MiniGaffer::MiniMeshCurvature, MiniGaffer::TypeId::MeshCurvatureTypeId, ObjectProcessor );

		Gaffer::StringPlug *positionPlug();
		const Gaffer::StringPlug *positionPlug() const;

		Gaffer::StringPlug *normalPlug();
		const Gaffer::StringPlug *normalPlug() const;

		Gaffer::StringPlug *curvaturePlug();
		const Gaffer::StringPlug *curvaturePlug() const;

		Gaffer::FloatPlug *scalePlug();
		const Gaffer::FloatPlug *scalePlug() const;

		Gaffer::BoolPlug *normalizePlug();
		const Gaffer::BoolPlug *normalizePlug() const;

	protected :

		bool affectsProcessedObject( const Gaffer::Plug *input ) const override;
		void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
		Gaffer::ValuePlug::CachePolicy processedObjectComputeCachePolicy() const override;

	private :

		static size_t g_firstPlugIndex;

};

IE_CORE_DECLAREPTR(MiniMeshCurvature )

} // namespace MiniGaffer
