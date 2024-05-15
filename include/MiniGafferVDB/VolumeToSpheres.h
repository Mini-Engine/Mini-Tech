#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{

	class MINIGAFFERVDB_API VolumeToSpheres : public GafferScene::SceneElementProcessor
	{

	public :

		VolumeToSpheres(const std::string &name = defaultName<VolumeToSpheres>() );
		~VolumeToSpheres();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGafferVDB::VolumeToSpheres, TypeId::VolumeToSpheresTypeId, GafferScene::SceneElementProcessor );
		
		Gaffer::StringPlug *gridsPlug();
		const Gaffer::StringPlug *gridsPlug() const;

        Gaffer::IntPlug *minSpheresPlug();
        const Gaffer::IntPlug *minSpheresPlug() const;

        Gaffer::IntPlug *maxSpheresPlug();
        const Gaffer::IntPlug *maxSpheresPlug() const;

        Gaffer::BoolPlug *overlappingPlug();
        const Gaffer::BoolPlug *overlappingPlug() const;

        Gaffer::FloatPlug *minRadiusPlug();
        const Gaffer::FloatPlug *minRadiusPlug() const;

        Gaffer::FloatPlug *maxRadiusPlug();
        const Gaffer::FloatPlug *maxRadiusPlug() const;

        Gaffer::FloatPlug *isoValuePlug();
        const Gaffer::FloatPlug *isoValuePlug() const;

        virtual void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		bool processesObject() const override;
		void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const override;

		bool processesBound() const override;
		void hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		Imath::Box3f computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const override;

	private:

		static size_t g_firstPlugIndex;
	};

	IE_CORE_DECLAREPTR( VolumeToSpheres )

}
