#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "Gaffer/NumericPlug.h"

namespace Gaffer
{
class StringPlug;
}

namespace MiniGafferVDB
{

class MINIGAFFERVDB_API MeshToLevelSet : public GafferScene::SceneElementProcessor
{

	public :

		MeshToLevelSet( const std::string &name=defaultName<MeshToLevelSet>() );
		~MeshToLevelSet() override;

		GAFFER_GRAPHCOMPONENT_DECLARE_TYPE( MiniGafferVDB::MeshToLevelSet, TypeId::MeshToLevelSetTypeId, GafferScene::SceneElementProcessor );

		Gaffer::StringPlug *gridPlug();
		const Gaffer::StringPlug *gridPlug() const;

		Gaffer::FloatPlug *voxelSizePlug();
		const Gaffer::FloatPlug *voxelSizePlug() const;

		Gaffer::FloatPlug *exteriorBandwidthPlug();
		const Gaffer::FloatPlug *exteriorBandwidthPlug() const;

		Gaffer::FloatPlug *interiorBandwidthPlug();
		const Gaffer::FloatPlug *interiorBandwidthPlug() const;

		void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		bool processesObject() const override;
		void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const override;

	private :

		static size_t g_firstPlugIndex;

};

IE_CORE_DECLAREPTR( MeshToLevelSet )

}