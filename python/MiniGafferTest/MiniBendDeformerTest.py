import math

import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferSceneTest

import MiniGaffer

class MiniBendDeformerTest( GafferSceneTest.SceneTestCase ) :

    def __scene( self, points, normals = None ) :

        script = Gaffer.ScriptNode()

        prim = IECoreScene.PointsPrimitive( IECore.V3fVectorData( points ) )
        if normals is not None :
            prim["N"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( normals ) )

        script["source"] = GafferScene.ObjectToScene()
        script["source"]["object"].setValue( prim )
        script["source"]["name"].setValue( "thing" )

        script["filter"] = GafferScene.PathFilter()
        script["filter"]["paths"].setValue( IECore.StringVectorData( [ "/thing" ] ) )

        script["bend"] = MiniGaffer.MiniBendDeformer()
        script["bend"]["in"].setInput( script["source"]["out"] )
        script["bend"]["filter"].setInput( script["filter"]["out"] )

        return script

    def __result( self, script ) :

        return script["bend"]["out"].object( "/thing" )

    def __assertAlmostEqual( self, a, b, places = 4 ) :

        self.assertTrue( ( a - b ).length() < 10 ** -places, "{} != {}".format( a, b ) )

    def testConstruct( self ) :

        node = MiniGaffer.MiniBendDeformer()
        self.assertEqual( node.getName(), "MiniBendDeformer" )
        self.assertEqual( node["curvature"].getValue(), 0 )

    def testZeroCurvatureDoesNothing( self ) :

        script = self.__scene( [ imath.V3f( 1, 0.5, 0 ) ] )
        self.assertEqual( list( self.__result( script )["P"].data ), [ imath.V3f( 1, 0.5, 0 ) ] )

    def testPointOnAxisBendsOntoArc( self ) :

        # Curvature 1 means a circle of radius 1 centred on x = 1.
        script = self.__scene( [ imath.V3f( 0, 0.5, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )

        p = self.__result( script )["P"].data[0]
        self.__assertAlmostEqual( p, imath.V3f( 1 - math.cos( 0.5 ), math.sin( 0.5 ), 0 ) )

        # It stays on the circle.
        self.assertAlmostEqual( ( p - imath.V3f( 1, 0, 0 ) ).length(), 1, places = 4 )

    def testPointsAtOriginAndOffAxis( self ) :

        script = self.__scene( [ imath.V3f( 0, 0, 0 ), imath.V3f( 0.5, 0, 0.7 ) ] )
        script["bend"]["curvature"].setValue( 1 )
        result = self.__result( script )["P"].data

        # The origin does not move, and z is untouched.
        self.__assertAlmostEqual( result[0], imath.V3f( 0 ) )
        self.assertAlmostEqual( result[1].z, 0.7, places = 5 )
        # y = 0 means no rotation, so x is unchanged.
        self.assertAlmostEqual( result[1].x, 0.5, places = 5 )

    def testOffAxisPointRotatesAboutCentre( self ) :

        script = self.__scene( [ imath.V3f( 0.5, 1, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )

        p = self.__result( script )["P"].data[0]
        # Radius from the arc centre ( 1, 0 ) is preserved, here 0.5.
        self.assertAlmostEqual( ( p - imath.V3f( 1, 0, 0 ) ).length(), 0.5, places = 4 )

    def testNegativeCurvatureBendsTheOtherWay( self ) :

        script = self.__scene( [ imath.V3f( 0, 0.5, 0 ) ] )
        script["bend"]["curvature"].setValue( -1 )
        self.assertTrue( self.__result( script )["P"].data[0].x < 0 )

    def testBoundsContinueStraight( self ) :

        script = self.__scene( [ imath.V3f( 0, 1, 0 ), imath.V3f( 0, 3, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )
        script["bend"]["highBound"].setValue( 1 )

        result = self.__result( script )["P"].data
        end = imath.V3f( 1 - math.cos( 1 ), math.sin( 1 ), 0 )
        self.__assertAlmostEqual( result[0], end )

        # Two units beyond the bound continue along the tangent at the end of the arc.
        tangent = imath.V3f( math.sin( 1 ), math.cos( 1 ), 0 )
        self.__assertAlmostEqual( result[1], end + tangent * 2 )

    def testLowBound( self ) :

        script = self.__scene( [ imath.V3f( 0, -3, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )
        script["bend"]["lowBound"].setValue( -1 )

        end = imath.V3f( 1 - math.cos( -1 ), math.sin( -1 ), 0 )
        tangent = imath.V3f( math.sin( -1 ), math.cos( -1 ), 0 )
        self.__assertAlmostEqual( self.__result( script )["P"].data[0], end + tangent * -2 )

    def testEnvelopeBlends( self ) :

        script = self.__scene( [ imath.V3f( 0, 0.5, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )
        script["bend"]["envelope"].setValue( 1 )
        bent = self.__result( script )["P"].data[0]

        script["bend"]["envelope"].setValue( 0.5 )
        half = self.__result( script )["P"].data[0]
        self.__assertAlmostEqual( half, ( imath.V3f( 0, 0.5, 0 ) + bent ) / 2 )

        script["bend"]["envelope"].setValue( 0 )
        self.assertEqual( self.__result( script )["P"].data[0], imath.V3f( 0, 0.5, 0 ) )

    def testHandleMovesBendCentre( self ) :

        # Moving the handle up by 1 should make a point at y = 1 behave like y = 0 : no movement.
        script = self.__scene( [ imath.V3f( 0, 1, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )
        script["bend"]["handle"]["translate"].setValue( imath.V3f( 0, 1, 0 ) )
        self.__assertAlmostEqual( self.__result( script )["P"].data[0], imath.V3f( 0, 1, 0 ) )

    def testHandleRotationChangesBendDirection( self ) :

        # Rotating the handle 90 degrees about Y turns the bend towards Z.
        script = self.__scene( [ imath.V3f( 0, 0.5, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )
        script["bend"]["handle"]["rotate"].setValue( imath.V3f( 0, 90, 0 ) )

        p = self.__result( script )["P"].data[0]
        self.assertAlmostEqual( p.x, 0, places = 4 )
        self.assertAlmostEqual( abs( p.z ), 1 - math.cos( 0.5 ), places = 4 )

    def testNormalsAreRotated( self ) :

        script = self.__scene( [ imath.V3f( 0, 1, 0 ) ], [ imath.V3f( 1, 0, 0 ) ] )
        script["bend"]["curvature"].setValue( 1 )

        n = self.__result( script )["N"].data[0]
        self.__assertAlmostEqual( n, imath.V3f( math.cos( 1 ), -math.sin( 1 ), 0 ) )
        self.assertAlmostEqual( n.length(), 1, places = 4 )
