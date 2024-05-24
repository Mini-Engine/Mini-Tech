import Gaffer
import GafferUI
import os

import MiniGaffer
import MiniGafferUI


def __getReferencePath(filename):
	return "{}".format(filename)

def __miniCurvesRigidBind() :
	return Gaffer.Reference( "CurvesRigidBind" )

def __miniCurvesRigidBindPostCreator( node, menu ) :
	node.load(
		os.path.expandvars( __getReferencePath("curvesRigidBind.grf") )
	)

def __miniDeformTransform() :
	return Gaffer.Reference( "DeformTransform" )

def __miniDeformTransformPostCreator( node, menu ) :
	node.load(
		os.path.expandvars( __getReferencePath("makeDeformTransform.grf") )
	)

def __miniWindDeformer():
	return Gaffer.Reference( "WindDeformer" )

def __miniWindDeformerPostCreator(node, menu):
	node.load(
		os.path.expandvars( __getReferencePath("windDeformer.grf" ) )
	)

def __miniArnoldInitializer():
	return Gaffer.Reference( "ArnoldInitializer" )

def __miniArnoldInitializerPostCreator(node, menu):
	node.load(
		os.path.expandvars( __getReferencePath("arnoldInitializer.grf" ) )
	)

def __miniBubbleMask():
	return Gaffer.Reference( "BubbleMask" )

def __miniBubbleMaskPostCreator(node, menu):
	node.load(
		os.path.expandvars( __getReferencePath("bubbleMask.grf" ) )
	)

def __miniCameraFrustumPointCull():
	return Gaffer.Reference( "CameraFrustumPointCull" )

def __miniCameraFrustumPointCullPostCreator(node, menu):
	node.load(
		os.path.expandvars( __getReferencePath("cameraFrustumPointCull.grf" ) )
	)

def __miniCameraReader():
	return Gaffer.Reference( "CameraReader" )

def __miniCameraReaderPostCreator(node, menu):
	node.load(
		os.path.expandvars( __getReferencePath("cameraReader.grf" ) )
	)

def __miniDecimateCurves():
	return Gaffer.Reference( "DecimateCurves" )

def __miniDecimateCurvesPostCreator(node, menu):
	node.load(
		os.path.expandvars( __getReferencePath("decimateCurves.grf" ) )
	)

def __miniDecimatePoints():
	return Gaffer.Reference( "DecimatePoints" )

def __miniDecimatePointsPostCreator(node, menu):
	node.load(
		os.path.expandvars( __getReferencePath( "decimatePoints.grf" ) )
	)
def __miniManifold():
		return Gaffer.Reference( "Manifold" )

def __miniManifoldPostCreator(node, menu):
	node.load(__getReferencePath("manifold.grf" ))

def __miniOslRandom():
	return Gaffer.Reference( "OslRandom" )

def __miniOslRandomPostCreator(node, menu):
	node.load(__getReferencePath("oslRandom.grf" ))

def __miniMerge():
	return Gaffer.Reference( "MiniMerge")

def __miniMergePostCreator(node, menu):
	node.load(__getReferencePath("MiniMerge.grf" ))


nodeMenu = GafferUI.NodeMenu.acquire( application )

if os.environ.get("MINITECH_REF_NODES", False):

	nodeMenu.append(
		path = "/Mini/Scene/Curves/MakeDeformTransform",
		nodeCreator = __miniDeformTransform,
		postCreator = __miniDeformTransformPostCreator,
		searchText = "MakeDeformTransform"
	)

	nodeMenu.append(
		path = "/Mini/Scene/Curves/CurvesRigidBind",
		nodeCreator = __miniCurvesRigidBind,
		postCreator = __miniCurvesRigidBindPostCreator,
		searchText = "CurvesRigidBind"
	)

	nodeMenu.append(
		path = "/Mini/Scene/Curves/WindDeformer",
		nodeCreator = __miniWindDeformer,
		postCreator = __miniWindDeformerPostCreator,
		searchText = "WindDeformer"
	)

	nodeMenu.append(
		path = "/Mini/Render/ArnoldInitializer",
		nodeCreator = __miniArnoldInitializer,
		postCreator = __miniArnoldInitializerPostCreator,
		searchText = "WindDeformer"
	)

	nodeMenu.append(
		path = "/Mini/Shader/BubbleMask",
		nodeCreator = __miniBubbleMask,
		postCreator = __miniBubbleMaskPostCreator,
		searchText = "BubbleMask"
	)

	nodeMenu.append(
		path = "/Mini/Scene/CameraFrustumPointCull",
		nodeCreator = __miniCameraFrustumPointCull,
		postCreator = __miniCameraFrustumPointCullPostCreator,
		searchText = "CameraFrustumPointCull"
	)

	nodeMenu.append(
		path = "/Mini/Scene/CameraReader",
		nodeCreator = __miniCameraReader,
		postCreator = __miniCameraReaderPostCreator,
		searchText = "CameraReader"
	)

	nodeMenu.append(
		path = "/Mini/Scene/Curves/DecimateCurves",
		nodeCreator = __miniDecimateCurves,
		postCreator = __miniDecimateCurvesPostCreator,
		searchText = "DecimateCurves"
	)

	nodeMenu.append(
		path = "/Mini/Scene/Points/DecimatePoints",
		nodeCreator = __miniDecimatePoints,
		postCreator = __miniDecimatePointsPostCreator,
		searchText = "DecimatePoints"
	)

	nodeMenu.append(
		path = "/Mini/Shader/Manifold",
		nodeCreator = __miniManifold,
		postCreator = __miniManifoldPostCreator,
		searchText = "Manifold"
	)

	nodeMenu.append(
		path = "/Mini/Shader/OslRandom",
		nodeCreator = __miniOslRandom,
		postCreator = __miniOslRandomPostCreator,
		searchText = "OslRandom"
	)

nodeMenu.append(
	path = "/Mini/Scene/MiniMerge",
	nodeCreator = __miniMerge,
	postCreator = __miniMergePostCreator,
	searchText = "MiniMerge"
)

nodeMenu.append( "/Mini/Scene/Curves/ResampleCurves", MiniGaffer.MiniResampleCurves, searchText = "ResampleCurves" )
nodeMenu.append( "/Mini/Scene/Curves/MeasureCurves", MiniGaffer.MiniMeasureCurves, searchText = "MeasureCurves" )
nodeMenu.append( "/Mini/Scene/MergeGeometry", MiniGaffer.MiniMergeGeometry, searchText = "MergeGeometry" )
nodeMenu.append( "/Mini/Scene/SubdivideMesh", MiniGaffer.MiniSubdivideMesh, searchText = "SubdivideMesh" )
nodeMenu.append( "/Mini/Physics/Bullet", MiniGaffer.MiniBullet, searchText = "Bullet" )
nodeMenu.append( "/Mini/Scene/Curves/GrowCurves", MiniGaffer.MiniGrowCurves, searchText = "GrowCurves")

nodeMenu.append( "/Mini/Scene/CurvesTypes", MiniGaffer.MiniCurvesType, searchText = "CurvesType" )
nodeMenu.append( "/Mini/Scene/PointDeformer", MiniGaffer.MiniPointDeformer, searchText = "PointDeformer" )
nodeMenu.append( "/Mini/Scene/PointBind", MiniGaffer.MiniPointBind, searchText = "PointBind" )

nodeMenu.append( "/Mini/Scene/MeshCurvature", MiniGaffer.MiniMeshCurvature, searchText = "MeshCurvature" )
nodeMenu.append( "/Mini/Scene/BlurPrimitiveVariables", MiniGaffer.MiniBlurPrimitiveVariables, searchText = "BlurPrimitiveVariables" )



import MiniGafferVDB
import MiniGafferVDBUI

nodeMenu.append( "/Mini/VDB/Offset Level Set", MiniGafferVDB.OffsetLevelSet, searchText = "OffsetLevelSet" )
nodeMenu.append( "/Mini/VDB/Points Grid To Points", MiniGafferVDB.PointsGridToPoints, searchText = "PointsGridToPoints" )
nodeMenu.append( "/Mini/VDB/Delete Grids", MiniGafferVDB.DeleteGrids, searchText = "DeleteGrids" )
nodeMenu.append( "/Mini/VDB/Advect Grids", MiniGafferVDB.AdvectGrids, searchText = "AdvectGrids" )
nodeMenu.append( "/Mini/VDB/Advect Points", MiniGafferVDB.AdvectPoints, searchText = "AdvectPoints" )
nodeMenu.append( "/Mini/VDB/Math Op Grids", MiniGafferVDB.MathOpGrids, searchText = "MathOpGrids" )
nodeMenu.append( "/Mini/VDB/Statistics Grids", MiniGafferVDB.StatisticsGrids, searchText = "StatisticsGrids" )
nodeMenu.append( "/Mini/VDB/CSG LevelSets", MiniGafferVDB.CSGLevelSets, searchText = "CSGLevelSets" )
nodeMenu.append( "/Mini/VDB/Transform Grids", MiniGafferVDB.TransformGrids, searchText = "TransformGrids" )
nodeMenu.append( "/Mini/VDB/Points To LevelSet", MiniGafferVDB.PointsToLevelSet, searchText = "PointsToLevelSet" )
nodeMenu.append( "/Mini/VDB/VDB Object", MiniGafferVDB.VDBObject, searchText = "VDBObject" )
nodeMenu.append( "/Mini/VDB/Sample Grids", MiniGafferVDB.SampleGrids, searchText = "SampleGrids" )
nodeMenu.append( "/Mini/VDB/Filter Grids", MiniGafferVDB.FilterGrids, searchText = "FilterGrids" )
nodeMenu.append( "/Mini/VDB/Measure LevelSet", MiniGafferVDB.MeasureLevelSet, searchText = "MeasureLevelSet" )
nodeMenu.append( "/Mini/VDB/Filter LevelSet", MiniGafferVDB.FilterLevelSet, searchText = "FilterLevelSet" )
nodeMenu.append( "/Mini/VDB/Volume To Spheres", MiniGafferVDB.VolumeToSpheres, searchText = "VolumeToSpheres" )
nodeMenu.append( "/Mini/VDB/Clip Grids", MiniGafferVDB.ClipGrids, searchText = "ClipGrids" )
nodeMenu.append( "/Mini/VDB/LevelSet To Fog", MiniGafferVDB.LevelSetToFog, searchText="LevelSetToFog")
nodeMenu.append( "/Mini/VDB/Segment LevelSets", MiniGafferVDB.SegmentLevelSets, searchText="SegmentLevelSets")
nodeMenu.append( "/Mini/VDB/Intersect LevelSet", MiniGafferVDB.IntersectLevelSet, searchText="IntersectLevelSet")
nodeMenu.append( "/Mini/VDB/Morph LevelSets", MiniGafferVDB.MorphLevelSets, searchText="MorphLevelSets")
nodeMenu.append( "/Mini/VDB/Composite Grids", MiniGafferVDB.CompositeGrids, searchText="CompositeGrids")
nodeMenu.append( "/Mini/VDB/Platonic LevelSet", MiniGafferVDB.PlatonicLevelSet, searchText="PlatonicLevelSet")
nodeMenu.append( "/Mini/VDB/Sphere LevelSet", MiniGafferVDB.SphereLevelSet, searchText="SphereLevelSet")
nodeMenu.append( "/Mini/VDB/Mask Grids", MiniGafferVDB.MaskGrids, searchText="MaskGrids")