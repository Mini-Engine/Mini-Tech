#include <Mini/MiniPointDeformer.h>

#include "boost/python.hpp"

#include "IECorePython/RunTimeTypedBinding.h"

#include "GafferBindings/DependencyNodeBinding.h"
#include "IECore/MessageHandler.h"

#include "Mini/MiniResampleCurves.h"
#include "Mini/MiniMeasureCurves.h"
#include "Mini/MiniMergeGeometry.h"
#include "Mini/MiniSubdivideMesh.h"
#include "Mini/MiniBlurPrimitiveVariables.h"
#include "Mini/MiniMeshCurvature.h"
#include "Mini/MiniCurvesType.h"
#include "Mini/MiniBullet.h"
#include "Mini/MiniPointDeformer.h"
#include "Mini/MiniPointBind.h"

using namespace boost::python;

BOOST_PYTHON_MODULE( _MiniGaffer )
{
    typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniResampleCurves> MiniGafferResampleCurvesWrapper;
    GafferBindings::DependencyNodeClass<MiniGaffer::MiniResampleCurves, MiniGafferResampleCurvesWrapper>();

    typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniMeasureCurves> MiniGafferMeasureCurvesWrapper;
    GafferBindings::DependencyNodeClass<MiniGaffer::MiniMeasureCurves, MiniGafferMeasureCurvesWrapper>();

    typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniMergeGeometry> MiniGafferMergeGeometryWrapper;
    GafferBindings::DependencyNodeClass<MiniGaffer::MiniMergeGeometry, MiniGafferMergeGeometryWrapper>();

    typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniSubdivideMesh> MiniGafferSubdivideMeshWrapper;
    GafferBindings::DependencyNodeClass<MiniGaffer::MiniSubdivideMesh, MiniGafferSubdivideMeshWrapper>();

    typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniBullet> MiniBulletWrapper;
    GafferBindings::DependencyNodeClass<MiniGaffer::MiniBullet, MiniBulletWrapper>();

    typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniCurvesType> MiniGafferCurvesTypeWrapper;
    GafferBindings::DependencyNodeClass<MiniGaffer::MiniCurvesType, MiniGafferCurvesTypeWrapper>();

	typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniPointDeformer> MiniGafferPointDeformerWrapper;
	GafferBindings::DependencyNodeClass<MiniGaffer::MiniPointDeformer, MiniGafferPointDeformerWrapper>();

	typedef GafferBindings::DependencyNodeWrapper<MiniGaffer::MiniPointBind> MiniGafferPointBindWrapper;
	GafferBindings::DependencyNodeClass<MiniGaffer::MiniPointBind, MiniGafferPointBindWrapper>();

    GafferBindings::DependencyNodeClass<MiniGaffer::MiniMeshCurvature>();
	{
		scope s = GafferBindings::DependencyNodeClass<MiniGaffer::MiniBlurPrimitiveVariables>();
		enum_<MiniGaffer::MiniBlurPrimitiveVariables::Weighting>("Weighting" )
			.value( "Uniform", MiniGaffer::MiniBlurPrimitiveVariables::Weighting::Uniform )
			.value( "EdgeLength", MiniGaffer::MiniBlurPrimitiveVariables::Weighting::EdgeLength )
		;
	}

}
