#include "MiniGafferVDB/CSGLevelSets.h"
#include "GafferVDB/Interrupter.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/Composite.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;
using namespace GafferScene;

IE_CORE_DEFINERUNTIMETYPED(CSGLevelSets );

size_t CSGLevelSets::g_firstPlugIndex = 0;

// todo rename to CSGLevelSets

CSGLevelSets::CSGLevelSets(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild(g_firstPlugIndex);

	addChild( new ScenePlug( "in", Gaffer::Plug::In ) );

	addChild( new StringPlug( "vdbLocation", Plug::In, "/vdb" ) );
	addChild( new StringPlug( "grid", Plug::In, "surface" ) );
    addChild( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
	addChild( new IntPlug( "operation", Plug::In, 0) );

}

CSGLevelSets::~CSGLevelSets()
{
}

GafferScene::ScenePlug *CSGLevelSets::otherPlug()
{
	return  getChild<ScenePlug>( g_firstPlugIndex );
}

const GafferScene::ScenePlug *CSGLevelSets::otherPlug() const
{
	return  getChild<ScenePlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *CSGLevelSets::vdbLocationPlug()
{
	return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

const Gaffer::StringPlug *CSGLevelSets::vdbLocationPlug() const
{
	return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

Gaffer::StringPlug *CSGLevelSets::gridPlug()
{
	return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *CSGLevelSets::gridPlug() const
{
	return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::StringPlug *CSGLevelSets::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::StringPlug *CSGLevelSets::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 3 );
}

Gaffer::IntPlug *CSGLevelSets::operationPlug()
{
	return  getChild<IntPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::IntPlug *CSGLevelSets::operationPlug() const
{
	return  getChild<IntPlug>( g_firstPlugIndex + 4 );
}

void CSGLevelSets::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if(
	    input == operationPlug() ||
	    input->parent() == otherPlug() ||
	    input == gridPlug() ||
	    input == vdbLocationPlug() ||
	    input == outputGridPlug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->boundPlug() );
	}
}

bool CSGLevelSets::processesObject() const
{
	return true;
}

void CSGLevelSets::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedObject( path, context, h );

	ScenePlug::ScenePath otherPath;
	ScenePlug::stringToPath( vdbLocationPlug()->getValue(), otherPath );
	h.append( operationPlug()->hash() );
	h.append( otherPlug()->objectHash( otherPath ) );
	h.append( gridPlug()->hash() );
	h.append( vdbLocationPlug()->hash() );
	h.append( outputGridPlug()->hash() );
}

template<template<typename> class F>
class Dispatcher
{
public:

	openvdb::GridBase::Ptr operator()( openvdb::GridBase::Ptr grid ) const
	{
		if ( grid->valueType() == openvdb::typeNameAsString<double>() )
		{
			F<openvdb::DoubleGrid> f;
			auto p = f( grid );
			return p;
		}

		return nullptr;
	}

	openvdb::GridBase::Ptr operator()( openvdb::GridBase::ConstPtr grid ) const
	{
		if ( grid->valueType() == openvdb::typeNameAsString<double>() )
		{
			F<openvdb::DoubleGrid> f;
			auto p = f( grid );
			return p;
		}
		return nullptr;
	}

	openvdb::GridBase::Ptr operator()( openvdb::GridBase::Ptr gridA, openvdb::GridBase::Ptr gridB  ) const
	{
		if ( gridA->valueType() == openvdb::typeNameAsString<double>() && gridB->valueType() == openvdb::typeNameAsString<double>() )
		{
			F<openvdb::DoubleGrid> f;
			auto p = f( gridA, gridB );
			return p;
		}
		return nullptr;
	}

	openvdb::GridBase::Ptr operator()( openvdb::GridBase::ConstPtr gridA, openvdb::GridBase::ConstPtr gridB  ) const
	{
		if ( gridA->valueType() == openvdb::typeNameAsString<double>() && gridB->valueType() == openvdb::typeNameAsString<double>() )
		{
			F<openvdb::DoubleGrid> f;
			auto p = f( openvdb::GridBase::constGrid<openvdb::DoubleGrid>( gridA ), openvdb::GridBase::constGrid<openvdb::DoubleGrid>( gridB ) );
			return p;
		}
		else if ( gridA->valueType() == openvdb::typeNameAsString<float>() && gridB->valueType() == openvdb::typeNameAsString<float>() )
		{
			F<openvdb::FloatGrid> f;
			auto p = f( openvdb::GridBase::constGrid<openvdb::FloatGrid>( gridA ), openvdb::GridBase::constGrid<openvdb::FloatGrid>( gridB ) );
			return p;
		}
		return nullptr;
	}
};

template<typename VDBGridType>
class UnionFunctor
{
public:
	typedef typename VDBGridType::Ptr Ptr;
	typedef typename VDBGridType::ConstPtr ConstPtr;

	Ptr operator() (ConstPtr gridA, ConstPtr gridB )
	{
		Ptr copyOfGridA = gridA->deepCopy();
		Ptr copyOfGridB = gridB->deepCopy();

		openvdb::tools::csgUnion(*copyOfGridA, *copyOfGridB);

		return copyOfGridA;
	}
};

template<typename VDBGridType>
class IntersectionFunctor
{
public:
	typedef typename VDBGridType::Ptr Ptr;
	typedef typename VDBGridType::ConstPtr ConstPtr;

	Ptr operator() (ConstPtr gridA, ConstPtr gridB )
	{
		Ptr copyOfGridA = gridA->deepCopy();
		Ptr copyOfGridB = gridB->deepCopy();

		openvdb::tools::csgIntersection(*copyOfGridA, *copyOfGridB);

		return copyOfGridA;
	}
};

template<typename VDBGridType>
class DifferenceFunctor
{
public:
	typedef typename VDBGridType::Ptr Ptr;
	typedef typename VDBGridType::ConstPtr ConstPtr;

	Ptr operator() (ConstPtr gridA, ConstPtr gridB )
	{
		Ptr copyOfGridA = gridA->deepCopy();
		Ptr copyOfGridB = gridB->deepCopy();

		openvdb::tools::csgDifference(*copyOfGridA, *copyOfGridB);

		return copyOfGridA;
	}
};


IECore::ConstObjectPtr CSGLevelSets::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	const VDBObject *vdbObjectA = runTimeCast<const VDBObject>(inputObject.get());
	if( !vdbObjectA )
	{
		return inputObject;
	}

	ScenePlug::ScenePath p ;
	ScenePlug::stringToPath( vdbLocationPlug()->getValue(), p );

	ConstVDBObjectPtr vdbObjectB = runTimeCast<const VDBObject>( otherPlug()->object( p ) );

	if ( !vdbObjectB )
	{
		return inputObject;
	}
	const std::string gridName = gridPlug()->getValue();

    openvdb::FloatGrid::ConstPtr srcGridA =  openvdb::GridBase::constGrid<openvdb::FloatGrid>( vdbObjectA->findGrid( gridName ) );
	openvdb::FloatGrid::ConstPtr srcGridB =  openvdb::GridBase::constGrid<openvdb::FloatGrid>( vdbObjectB->findGrid( gridName ) );

	if ( !srcGridA || !srcGridB )
	{
		return inputObject;
	}

	VDBObjectPtr newVDBObject = vdbObjectA->copy();

	openvdb::FloatGrid::Ptr copyOfGridA = srcGridA->deepCopy();
	openvdb::FloatGrid::Ptr copyOfGridB = srcGridB->deepCopy();

    GafferVDB::Interrupter interrupter( context->canceller() );

    Context::EditableScope scope( context );
    scope.set( IECore::InternedString("grid"), &gridName );
    const std::string outGridName = context->substitute( outputGridPlug()->getValue() );

	switch(operationPlug()->getValue())
	{
		case 0:
		{
			Dispatcher<UnionFunctor> d;
            auto g = d( srcGridA, srcGridB );
            g->setName( outGridName );
			newVDBObject->insertGrid( g );
			break;
		}
		case 1:
		{
			Dispatcher<IntersectionFunctor> d;
            auto g = d( srcGridA, srcGridB );
            g->setName( outGridName );
            newVDBObject->insertGrid( g );
			break;
		}
		case 2:
		{
			Dispatcher<DifferenceFunctor> d;
            auto g = d( srcGridA, srcGridB );
            g->setName( outGridName );
            newVDBObject->insertGrid( g );
			break;
		}
		case 3:
		{
			Dispatcher<DifferenceFunctor> d;
            auto g = d( srcGridB, srcGridA );
            g->setName( outGridName );
            newVDBObject->insertGrid( g );
			break;
		}
		default:
			return inputObject;
	}


	return newVDBObject;
}

bool CSGLevelSets::processesBound() const
{
	return true;
}

void CSGLevelSets::hashProcessedBound(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedBound( path, context, h );

	gridPlug()->hash( h );
}

Imath::Box3f CSGLevelSets::computeProcessedBound(const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
	// todo calculate bounds from vdb grids
	return inputBound;
}


