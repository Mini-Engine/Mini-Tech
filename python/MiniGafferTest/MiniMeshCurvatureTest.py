import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferOSL

import GafferTest
import GafferSceneTest

import MiniGaffer

class MeshCurvatureTest( GafferSceneTest.SceneTestCase ) :

	def testConstruct( self ) :

		node = MiniGaffer.MiniMeshCurvature()
		self.assertEqual( node.getName(), "MiniMeshCurvature" )

	def testCompute( self ):

		s = GafferScene.Sphere()
		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		mesh = m["out"].object( "/sphere" )
		self.assertIn( "curvature", mesh )
		self.assertIsInstance( mesh["curvature"].data, IECore.FloatVectorData )

	def testSceneValidity( self ) :

		s = GafferScene.Sphere()
		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		self.assertSceneValid( m["out"] )

	def testDisabled( self ) :

		s = GafferScene.Sphere()
		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		self.assertIn( "curvature", m["out"].object( "/sphere" ) )

		m["enabled"].setValue( False )

		self.assertScenesEqual( s["out"], m["out"] )

	def testCurvaturePlug( self ) :

		s = GafferScene.Sphere()
		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		self.assertIn( "curvature", m["out"].object( "/sphere" ) )

		m["curvature"].setValue("myCurvature")
		self.assertIn( "myCurvature", m["out"].object( "/sphere" ) )
		self.assertNotIn( "curvature" , m["out"].object( "/sphere" ) )

	def testGlobalsPassThrough( self ) :

		s = GafferScene.Sphere()
		l = GafferSceneTest.TestLight()

		g = GafferScene.Group()
		g["in"][0].setInput( s["out"] )
		g["in"][1].setInput( l["out"] )

		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( g["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		self.assertEqual( m["in"]["globals"].hash(), m["out"]["globals"].hash() )
		self.assertEqual( m["in"]["globals"].getValue(), m["out"]["globals"].getValue() )

	def testPlaneHasZeroCurvature( self ) :

		p = GafferScene.Plane()
		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( p["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		expectedCurvature = 0.0

		for i in range( 0, 4 ) :
			self.assertAlmostEqual( m["out"].object( "/plane" )["curvature"].data[i], expectedCurvature, delta = 0.00001 )

	def testConvexeResult( self ) :

		s = GafferScene.Sphere()
		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		for i in range( 0, 762 ) :
			self.assertTrue( m["out"].object( "/sphere" )["curvature"].data[i] > 0.0  )

	def testConcaveResult( self ) :

		s = GafferScene.Sphere()

		n = GafferOSL.OSLShader()
		n.loadShader( "ObjectProcessing/InNormal" )

		mv = GafferOSL.OSLShader()
		mv.loadShader( "Maths/MultiplyVector" )
		mv["parameters"]["a"].setInput( n["out"]["value"] )
		mv["parameters"]["b"].setValue( imath.V3f( -1 ) )

		o = GafferOSL.OSLObject()
		o["in"].setInput( s["out"] )

		o["primitiveVariables"].addChild(
			Gaffer.NameValuePlug(
				"N", IECore.V3fData( imath.V3f( 0 ), IECore.GeometricData.Interpretation.Normal )
			)
		)
		o["primitiveVariables"][0]["value"].setInput( mv["out"]["out"] )

		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( o["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )
		o["filter"].setInput( f["out"] )

		for i in range( 0, 762 ) :
			self.assertTrue( m["out"].object( "/sphere" )["curvature"].data[i] < 0.0  )

	def testRightAngle( self ) :

		plane = IECoreScene.MeshPrimitive.createPlane(
			imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ),
			imath.V2i( 2 )
		)

		plane["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ imath.V3f( -0.5, -0.5, 0 ), imath.V3f( 0, -0.5, 0 ), imath.V3f( 0.5, -0.5, 0 ), imath.V3f( -0.5, 0, 0 ), imath.V3f( 0, 0, 0 ), imath.V3f( 0.5, 0, 0 ), imath.V3f( -0.5, 0, 0.5 ), imath.V3f( 0, 0, 0.5 ), imath.V3f( 0.5, 0, 0.5 ) ], IECore.GeometricData.Interpretation.Point )
		)

		plane["N"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ imath.V3f( 0, 0, -1 ), imath.V3f( 0, 0, -1 ), imath.V3f( 0, 0, -1 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ) ], IECore.GeometricData.Interpretation.Normal )
		)

		o = GafferScene.ObjectToScene()
		o["object"].setValue( plane )
		o["name"].setValue( "plane" )

		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( o["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		#print( m["out"].object( "/plane" )["curvature"].data[4] )

	def testCubeHasConstantCurvature( self ) :

		c = GafferScene.Cube()
		r = GafferScene.ResamplePrimitiveVariables()
		r["names"].setValue( "N" )
		r["in"].setInput( c["out"] )

		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( r["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		r["filter"].setInput( f["out"] )
		m["filter"].setInput( f["out"] )

		expectedCurvature = m["out"].object( "/cube" )["curvature"].data[0]
		for i in range( 1, 8 ) :
			self.assertAlmostEqual( m["out"].object( "/cube" )["curvature"].data[i], expectedCurvature, delta = 0.00001 )

	def testScalePlug( self ) :

		c = GafferScene.Cube()
		r = GafferScene.ResamplePrimitiveVariables()
		r["names"].setValue( "N" )
		r["in"].setInput( c["out"] )

		m = MiniGaffer.MiniMeshCurvature()
		m["in"].setInput( r["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		r["filter"].setInput( f["out"] )
		m["filter"].setInput( f["out"] )

		expectedCurvature = m["out"].object( "/cube" )["curvature"].data[0]
		for s in [ 0.5, 1.0, 2.0, 10.0 ] :
			m["scale"].setValue( s )
			for i in range( 0, 8 ) :
				self.assertAlmostEqual( m["out"].object( "/cube" )["curvature"].data[i], expectedCurvature * s, delta = 0.00001 )

if __name__ == "__main__":
	unittest.main()
