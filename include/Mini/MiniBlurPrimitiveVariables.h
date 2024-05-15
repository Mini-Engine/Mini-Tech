#pragma once

#include "TypeIds.h"

#include "GafferScene/Deformer.h"
#include "Gaffer/StringPlug.h"

namespace Gaffer
{

IE_CORE_FORWARDDECLARE( StringPlug )

} // namespace Gaffer

namespace MiniGaffer
{

/// Blurs primitive variables
class MiniBlurPrimitiveVariables : public GafferScene::Deformer
{

	public :

		MiniBlurPrimitiveVariables(const std::string &name=defaultName<MiniBlurPrimitiveVariables>() );
		~MiniBlurPrimitiveVariables() override;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION(MiniGaffer::MiniBlurPrimitiveVariables, MiniGaffer::TypeId::BlurPrimitiveVariablesTypeId, Deformer );

		enum Weighting
		{
			Uniform = 0,
			EdgeLength = 1,
		};

		Gaffer::IntPlug *weightingPlug();
		const Gaffer::IntPlug *weightingPlug() const;

		Gaffer::StringPlug *primitiveVariablesPlug();
		const Gaffer::StringPlug *primitiveVariablesPlug() const;

		Gaffer::StringPlug *positionPlug();
		const Gaffer::StringPlug *positionPlug() const;

		Gaffer::IntPlug *iterationsPlug();
		const Gaffer::IntPlug *iterationsPlug() const;

		Gaffer::FloatPlug *iterationScalePlug();
		const Gaffer::FloatPlug *iterationScalePlug() const;

		Gaffer::StringPlug *weightPlug();
		const Gaffer::StringPlug *weightPlug() const;

	protected :

		// bool processesObject() const override;
		bool affectsProcessedObject( const Gaffer::Plug *input) const override;
		void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const override;
		bool adjustBounds() const override;

	private :

		static size_t g_firstPlugIndex;

};

IE_CORE_DECLAREPTR(MiniBlurPrimitiveVariables )

} // namespace MiniGaffer
