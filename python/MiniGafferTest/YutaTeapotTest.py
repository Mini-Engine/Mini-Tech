import imath

import IECore
import IECoreScene

import Gaffer
import GafferTest
import GafferScene
import GafferSceneTest

import MiniGaffer

class YutaTeapotTest( GafferSceneTest.SceneTestCase ) :

    def testDefaults( self ) :

        teapot = MiniGaffer.YutaTeapot()
        self.assertEqual( teapot["name"].getValue(), "teapot" )
        self.assertSceneValid( teapot["out"] )

        mesh = teapot["out"].object( "/teapot" )
        self.assertIsInstance( mesh, IECoreScene.MeshPrimitive )
        self.assertTrue( mesh.arePrimitiveVariablesValid() )
        self.assertIn( "N", mesh )
        self.assertIn( "uv", mesh )
        self.assertEqual( mesh["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

        bound = mesh.bound()
        self.assertAlmostEqual( bound.min().y, 0, places = 4 )
        self.assertAlmostEqual( bound.max().y, 3.15, places = 4 )
        # Spout points down +X, the handle down -X.
        self.assertGreater( bound.max().x, 3.4 )
        self.assertLess( bound.min().x, -2.9 )

    def testPatchesAreWelded( self ) :

        teapot = MiniGaffer.YutaTeapot()
        teapot["divisions"].setValue( 4 )
        mesh = teapot["out"].object( "/teapot" )

        # Unwelded, every patch would have its own 5 x 5 grid of points.
        self.assertLess( mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 32 * 25 * 0.8 )

    def testNormalsPointOutwards( self ) :

        teapot = MiniGaffer.YutaTeapot()
        mesh = teapot["out"].object( "/teapot" )

        points = mesh["P"].data
        normals = mesh["N"].data
        centre = imath.V3f( 0, 1.2, 0 )
        # Check the widest part of the body, away from the handle and spout.
        checked = 0
        for p, n in zip( points, normals ) :
            radial = imath.V3f( p.x, 0, p.z )
            if abs( p.y - 0.9 ) < 0.05 and radial.length() > 1.95 and abs( p.z ) > 1.0 :
                self.assertGreater( n.dot( ( p - centre ).normalized() ), 0.5 )
                checked += 1
        self.assertGreater( checked, 0 )

        # The knob on the lid faces up and the middle of the bottom faces down.
        top = max( range( len( points ) ), key = lambda i : points[i].y )
        self.assertGreater( normals[top].y, 0.9 )
        bottom = min( range( len( points ) ), key = lambda i : points[i].length() )
        self.assertLess( normals[bottom].y, -0.9 )

    def testDivisions( self ) :

        teapot = MiniGaffer.YutaTeapot()
        teapot["divisions"].setValue( 2 )
        low = teapot["out"].object( "/teapot" ).numFaces()
        teapot["divisions"].setValue( 10 )
        high = teapot["out"].object( "/teapot" ).numFaces()
        self.assertGreater( high, low * 20 )

    def testSize( self ) :

        teapot = MiniGaffer.YutaTeapot()
        teapot["size"].setValue( 2 )
        self.assertAlmostEqual( teapot["out"].bound( "/teapot" ).max().y, 6.3, places = 4 )

    def testLidAndBottom( self ) :

        teapot = MiniGaffer.YutaTeapot()
        full = teapot["out"].object( "/teapot" ).numFaces()

        teapot["lid"].setValue( False )
        noLid = teapot["out"].object( "/teapot" )
        self.assertLess( noLid.numFaces(), full )
        self.assertLess( noLid.bound().max().y, 2.6 )

        teapot["lid"].setValue( True )
        teapot["bottom"].setValue( False )
        self.assertLess( teapot["out"].object( "/teapot" ).numFaces(), full )

    def testDirtyPropagation( self ) :

        teapot = MiniGaffer.YutaTeapot()
        for name in [ "size", "divisions", "lid", "bottom" ] :
            cs = GafferTest.CapturingSlot( teapot.plugDirtiedSignal() )
            teapot[name].setValue( not teapot[name].getValue() if name in ( "lid", "bottom" ) else teapot[name].getValue() + 1 )
            self.assertIn( teapot["out"]["object"], { x[0] for x in cs } )


if __name__ == "__main__":
    import unittest
    unittest.main()
