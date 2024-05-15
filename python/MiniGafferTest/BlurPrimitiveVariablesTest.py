import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene

import GafferTest
import GafferSceneTest

import MiniGaffer

class BlurPrimitiveVariablesTest( GafferSceneTest.SceneTestCase ) :

	def testConstruct( self ) :

		node = MiniGaffer.MiniBlurPrimitiveVariables()
		self.assertEqual( node.getName(), "MiniBlurPrimitiveVariables" )

	def testCompute( self ):

		s = GafferScene.Sphere()
		b = MiniGaffer.MiniBlurPrimitiveVariables()
		b["primitiveVariables"].setValue( "P" )
		b["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		b["filter"].setInput( f["out"] )

		self.assertNotEqual( b["out"].object( "/sphere" )["P"].data, s["out"].object( "/sphere" )["P"].data )

	def testSceneValidity( self ) :

		s = GafferScene.Sphere()
		m = MiniGaffer.MiniBlurPrimitiveVariables()
		m["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		self.assertSceneValid( m["out"] )

	def testDisabled( self ) :

		s = GafferScene.Sphere()
		b = MiniGaffer.MiniBlurPrimitiveVariables()
		b["primitiveVariables"].setValue( "P" )
		b["in"].setInput( s["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		b["filter"].setInput( f["out"] )

		self.assertNotEqual( b["out"].object( "/sphere" )["P"].data, s["out"].object( "/sphere" )["P"].data )

		b["enabled"].setValue( False )

		self.assertScenesEqual( s["out"], b["out"] )

	def testGlobalsPassThrough( self ) :

		s = GafferScene.Sphere()
		l = GafferSceneTest.TestLight()

		g = GafferScene.Group()
		g["in"][0].setInput( s["out"] )
		g["in"][1].setInput( l["out"] )

		m = MiniGaffer.MiniBlurPrimitiveVariables()
		m["in"].setInput( g["out"] )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		m["filter"].setInput( f["out"] )

		self.assertEqual( m["in"]["globals"].hash(), m["out"]["globals"].hash() )
		self.assertEqual( m["in"]["globals"].getValue(), m["out"]["globals"].getValue() )

	def testBlur( self ) :

		plane = IECoreScene.MeshPrimitive.createPlane(
			imath.Box2f( imath.V2f( -0.5 ), imath.V2f( 0.5 ) ),
			imath.V2i( 1 )
		)

		plane["vertexFloat"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( [
				x == 0 for x in range( 4 )
			] )
		)

		plane["vertexVector"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [
				imath.V3f( x == 0 ) for x in range( 4 )
			] )
		)

		plane["vertexV2f"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V2fVectorData( [
				imath.V2f( x == 0 ) for x in range( 4 )
			] )
		)

		plane["vertexColor"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.Color3fVectorData( [
				imath.Color3f( x == 0 ) for x in range( 4 )
			] )
		)

		plane["varyingColor"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Varying,
			plane["vertexColor"].data
		)

		mesh = GafferScene.ObjectToScene()
		mesh["object"].setValue( plane )
		mesh["name"].setValue( "mesh" )

		blur = MiniGaffer.MiniBlurPrimitiveVariables()
		blur["in"].setInput( mesh["out"] )
		blur["primitiveVariables"].setValue( "v*" )
		blur["iterations"].setValue( 2 )
		blur["iterationScale"].setValue( 0.5 )

		f = GafferScene.PathFilter()
		f["paths"].setValue( IECore.StringVectorData( [ "/*" ] ) )
		blur["filter"].setInput( f["out"] )

		self.assertNotEqual( plane["vertexColor"].data, blur["out"].object( "/mesh" )["vertexColor"].data )
		self.assertEqual( blur["out"].object( "/mesh" )["varyingColor"].data, blur["out"].object( "/mesh" )["vertexColor"].data )

		self.assertNotEqual( plane["vertexFloat"].data, blur["out"].object( "/mesh" )["vertexFloat"].data )

		self.assertEqual( blur["out"].object( "/mesh" )["vertexFloat"].data, IECore.FloatVectorData( [ 0.375, 0.25, 0.25, 0.125 ] ) )
		self.assertEqual( blur["out"].object( "/mesh" )["vertexColor"].data, IECore.Color3fVectorData( [ imath.Color3f( 0.375 ), imath.Color3f( 0.25 ), imath.Color3f( 0.25 ), imath.Color3f( 0.125 ) ] ) )
		self.assertEqual( blur["out"].object( "/mesh" )["varyingColor"].data, IECore.Color3fVectorData( [ imath.Color3f( 0.375 ), imath.Color3f( 0.25 ), imath.Color3f( 0.25 ), imath.Color3f( 0.125 ) ] ) )
		self.assertEqual( blur["out"].object( "/mesh" )["vertexVector"].data, IECore.V3fVectorData( [ imath.V3f( 0.375 ), imath.V3f( 0.25 ), imath.V3f( 0.25 ), imath.V3f( 0.125 ) ] ) )
		self.assertEqual( blur["out"].object( "/mesh" )["vertexV2f"].data, IECore.V2fVectorData( [ imath.V2f( 0.375 ), imath.V2f( 0.25 ), imath.V2f( 0.25 ), imath.V2f( 0.125 ) ] ) )

if __name__ == "__main__":
	unittest.main()
