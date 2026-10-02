import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferSceneTest

import MiniGaffer

class MiniBlendDeformerTest( GafferSceneTest.SceneTestCase ) :

    def __points( self, scene, path ) :

        return scene.object( path )["P"].data

    def __scene( self, basePoints, targetPoints, **extra ) :

        def prim( points ) :
            return IECoreScene.PointsPrimitive( IECore.V3fVectorData( points ) )

        # Base and target in an Object-per-location scene built with ObjectToScene.
        script = Gaffer.ScriptNode()

        script["base"] = GafferScene.ObjectToScene()
        base = script["base"]
        base["object"].setValue( prim( basePoints ) )
        base["name"].setValue( "base" )

        script["target"] = GafferScene.ObjectToScene()
        target = script["target"]
        target["object"].setValue( prim( targetPoints ) )
        target["name"].setValue( "target" )

        script["parent"] = GafferScene.Parent()
        parent = script["parent"]
        parent["parent"].setValue( "/" )
        parent["children"][0].setInput( target["out"] )
        parent["in"].setInput( base["out"] )

        script["filter"] = GafferScene.PathFilter()
        filter = script["filter"]
        filter["paths"].setValue( IECore.StringVectorData( [ "/base" ] ) )

        script["deformer"] = MiniGaffer.MiniBlendDeformer()
        deformer = script["deformer"]
        deformer["in"].setInput( parent["out"] )
        deformer["filter"].setInput( filter["out"] )

        return script

    def testConstruct( self ) :

        node = MiniGaffer.MiniBlendDeformer()
        self.assertEqual( node.getName(), "MiniBlendDeformer" )
        self.assertEqual( len( node["targets"] ), 0 )

    def testNoTargetsPassesThrough( self ) :

        script = self.__scene( [ imath.V3f( 0 ) ], [ imath.V3f( 1 ) ] )
        deformer = script["deformer"]
        self.assertEqual( self.__points( deformer["out"], "/base" ), self.__points( deformer["in"], "/base" ) )

    def testWeightBlendsTowardsTarget( self ) :

        script = self.__scene( [ imath.V3f( 0 ), imath.V3f( 1, 0, 0 ) ], [ imath.V3f( 0, 2, 0 ), imath.V3f( 1, 4, 0 ) ] )
        deformer = script["deformer"]
        deformer["targets"].resize( 1 )
        deformer["targets"][0]["location"].setValue( "/target" )

        deformer["targets"][0]["weight"].setValue( 0.5 )
        self.assertEqual( list( self.__points( deformer["out"], "/base" ) ), [ imath.V3f( 0, 1, 0 ), imath.V3f( 1, 2, 0 ) ] )

        # Weights are not clamped, as in Maya.
        deformer["targets"][0]["weight"].setValue( 2 )
        self.assertEqual( self.__points( deformer["out"], "/base" )[0], imath.V3f( 0, 4, 0 ) )

        deformer["targets"][0]["enabled"].setValue( False )
        self.assertEqual( self.__points( deformer["out"], "/base" )[0], imath.V3f( 0 ) )

    def testEnvelope( self ) :

        script = self.__scene( [ imath.V3f( 0 ) ], [ imath.V3f( 0, 2, 0 ) ] )
        deformer = script["deformer"]
        deformer["targets"].resize( 1 )
        deformer["targets"][0]["location"].setValue( "/target" )
        deformer["targets"][0]["weight"].setValue( 1 )

        deformer["envelope"].setValue( 0.25 )
        self.assertEqual( self.__points( deformer["out"], "/base" )[0], imath.V3f( 0, 0.5, 0 ) )

    def testTargetsAreSummed( self ) :

        script = self.__scene( [ imath.V3f( 0 ) ], [ imath.V3f( 0, 2, 0 ) ] )
        deformer = script["deformer"]
        deformer["targets"].resize( 2 )
        for target in deformer["targets"] :
            target["location"].setValue( "/target" )
            target["weight"].setValue( 0.5 )

        self.assertEqual( self.__points( deformer["out"], "/base" )[0], imath.V3f( 0, 2, 0 ) )

    def testBaseLocation( self ) :

        # Target offset is measured from the explicit base ( here the same as the target ), so no change.
        script = self.__scene( [ imath.V3f( 0 ) ], [ imath.V3f( 0, 2, 0 ) ] )
        deformer = script["deformer"]
        deformer["baseLocation"].setValue( "/target" )
        deformer["targets"].resize( 1 )
        deformer["targets"][0]["location"].setValue( "/target" )
        deformer["targets"][0]["weight"].setValue( 1 )

        self.assertEqual( self.__points( deformer["out"], "/base" )[0], imath.V3f( 0 ) )

    def testMask( self ) :

        script = self.__scene( [ imath.V3f( 0 ), imath.V3f( 0 ) ], [ imath.V3f( 0, 2, 0 ), imath.V3f( 0, 2, 0 ) ] )
        deformer = script["deformer"]
        deformer["targets"].resize( 1 )
        deformer["targets"][0]["location"].setValue( "/target" )
        deformer["targets"][0]["weight"].setValue( 1 )

        # Add the mask to the base geometry.
        base = script["base"]
        points = base["object"].getValue()
        points["mask"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 0.5 ] ) )
        base["object"].setValue( points )

        deformer["maskPrimVar"].setValue( "mask" )
        result = self.__points( deformer["out"], "/base" )
        self.assertEqual( result[0], imath.V3f( 0, 2, 0 ) )
        self.assertEqual( result[1], imath.V3f( 0, 1, 0 ) )

    def testMismatchedPointCountErrors( self ) :

        script = self.__scene( [ imath.V3f( 0 ) ], [ imath.V3f( 0 ), imath.V3f( 1 ) ] )
        deformer = script["deformer"]
        deformer["targets"].resize( 1 )
        deformer["targets"][0]["location"].setValue( "/target" )
        deformer["targets"][0]["weight"].setValue( 1 )

        with self.assertRaisesRegex( Exception, "has 2 points but 1 are required" ) :
            deformer["out"].object( "/base" )

    def testMissingTargetErrors( self ) :

        script = self.__scene( [ imath.V3f( 0 ) ], [ imath.V3f( 0 ) ] )
        deformer = script["deformer"]
        deformer["targets"].resize( 1 )
        deformer["targets"][0]["location"].setValue( "/nothing" )
        deformer["targets"][0]["weight"].setValue( 1 )

        with self.assertRaisesRegex( Exception, "nothing" ) :
            deformer["out"].object( "/base" )

    def testHashChangesWithWeight( self ) :

        script = self.__scene( [ imath.V3f( 0 ) ], [ imath.V3f( 0, 2, 0 ) ] )
        deformer = script["deformer"]
        deformer["targets"].resize( 1 )
        deformer["targets"][0]["location"].setValue( "/target" )
        deformer["targets"][0]["weight"].setValue( 0.5 )
        h = deformer["out"].objectHash( "/base" )
        deformer["targets"][0]["weight"].setValue( 0.6 )
        self.assertNotEqual( deformer["out"].objectHash( "/base" ), h )
