import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferSceneTest

import MiniGaffer

class MiniDustHitTest( GafferSceneTest.SceneTestCase ) :

    # A unit cube that falls onto a floor : it is clear of the floor until
    # frame 10, and touching it from then on.
    def __scene( self ) :

        script = Gaffer.ScriptNode()

        script["floor"] = GafferScene.Plane()
        script["floor"]["name"].setValue( "floor" )
        script["floor"]["dimensions"].setValue( imath.V2f( 10 ) )
        script["floor"]["transform"]["rotate"]["x"].setValue( -90 )

        script["cube"] = GafferScene.Cube()
        script["cube"]["name"].setValue( "cube" )
        script["cube"]["dimensions"].setValue( imath.V3f( 1 ) )

        # Cube centre is at y = 0.5 + 0.1 * ( 10 - frame ) : moving 1 unit per
        # 10 frames, resting on the floor at frame 10.
        script["cubeExpression"] = Gaffer.Expression()
        script["cubeExpression"].setExpression(
            'parent["cube"]["transform"]["translate"]["y"] = 0.5 + max( 0.1 * ( 10 - context.getFrame() ), 0 )',
            "python"
        )

        script["group"] = GafferScene.Group()
        script["group"]["in"][0].setInput( script["floor"]["out"] )
        script["group"]["in"][1].setInput( script["cube"]["out"] )

        script["filter"] = GafferScene.PathFilter()
        script["filter"]["paths"].setValue( IECore.StringVectorData( [ "/" ] ) )

        script["dust"] = MiniGaffer.MiniDustHit()
        script["dust"]["in"].setInput( script["group"]["out"] )
        script["dust"]["filter"].setInput( script["filter"]["out"] )
        script["dust"]["colliders"].setValue( "/group/floor /group/cube" )
        script["dust"]["contactDistance"].setValue( 0.05 )

        return script

    def __points( self, script, frame ) :

        with Gaffer.Context( script.context() ) as c :
            c.setFrame( frame )
            return script["dust"]["out"].object( "/dust" )

    def testConstruct( self ) :

        node = MiniGaffer.MiniDustHit()
        self.assertEqual( node.getName(), "MiniDustHit" )
        self.assertEqual( node["name"].getValue(), "dust" )

    def testNoCollidersMakesNoDust( self ) :

        script = self.__scene()
        script["dust"]["colliders"].setValue( "" )
        self.assertEqual( self.__points( script, 20 ).numPoints, 0 )

    def testNoDustBeforeContact( self ) :

        script = self.__scene()
        self.assertEqual( self.__points( script, 1 ).numPoints, 0 )
        self.assertEqual( self.__points( script, 8 ).numPoints, 0 )

    def testDustAppearsOnContact( self ) :

        script = self.__scene()
        script["dust"]["countPerHit"].setValue( 50 )

        points = self.__points( script, 10 )
        self.assertEqual( points.numPoints, 50 )

        # Right on the hit frame the dust is at the contact : the bottom of the cube.
        for p in points["P"].data :
            self.assertLess( abs( p.y ), 0.5 )
            self.assertLess( abs( p.x ), 1.5 )
            self.assertLess( abs( p.z ), 1.5 )

        for name in ( "v", "width", "opacity", "age", "Cd", "id" ) :
            self.assertIn( name, points )

    def testDustAgesAndFades( self ) :

        script = self.__scene()
        script["dust"]["lifetime"].setValue( 20 )

        young = self.__points( script, 10 )
        old = self.__points( script, 20 )
        self.assertEqual( young.numPoints, old.numPoints )

        self.assertLess( max( young["age"].data ), min( old["age"].data ) )
        self.assertGreater( min( young["width"].data ), 0 )
        self.assertGreater( min( old["width"].data ), min( young["width"].data ) )
        self.assertGreater( max( young["opacity"].data ), max( old["opacity"].data ) )

    def testDustDiesAfterLifetime( self ) :

        script = self.__scene()
        script["dust"]["lifetime"].setValue( 5 )
        self.assertEqual( self.__points( script, 40 ).numPoints, 0 )

    def testOnlyOneHitWhileResting( self ) :

        script = self.__scene()
        script["dust"]["lifetime"].setValue( 100 )
        self.assertEqual( self.__points( script, 50 ).numPoints, 200 )

    def testMinSpeedIgnoresSlowContacts( self ) :

        # The cube moves at 10 units per second at 24fps by 0.1 per frame = 2.4.
        script = self.__scene()
        script["dust"]["minSpeed"].setValue( 1 )
        self.assertEqual( self.__points( script, 10 ).numPoints, 200 )

        script["dust"]["minSpeed"].setValue( 100 )
        self.assertEqual( self.__points( script, 10 ).numPoints, 0 )

    def testCountBySpeed( self ) :

        script = self.__scene()
        script["dust"]["countPerHit"].setValue( 0 )
        script["dust"]["countBySpeed"].setValue( 10 )
        self.assertGreater( self.__points( script, 10 ).numPoints, 0 )

    def testDeterministicAndOrderIndependent( self ) :

        script = self.__scene()
        first = self.__points( script, 14 )
        self.__points( script, 3 )
        self.__points( script, 30 )
        self.assertEqual( self.__points( script, 14 ), first )

        script["dust"]["seed"].setValue( 1 )
        self.assertNotEqual( self.__points( script, 14 )["P"].data, first["P"].data )

    def testBranchLocation( self ) :

        script = self.__scene()
        self.assertEqual( script["dust"]["out"].childNames( "/" ), IECore.InternedStringVectorData( [ "group", "dust" ] ) )

        script["dust"]["name"].setValue( "puff" )
        self.assertEqual( script["dust"]["out"].childNames( "/" ), IECore.InternedStringVectorData( [ "group", "puff" ] ) )

        # The input scene is passed through untouched.
        for path in ( "/group", "/group/cube", "/group/floor" ) :
            self.assertEqual( script["dust"]["out"].object( path ), script["group"]["out"].object( path ) )
            self.assertEqual( script["dust"]["out"].childNames( path ), script["group"]["out"].childNames( path ) )
            self.assertEqual( script["dust"]["out"].transform( path ), script["group"]["out"].transform( path ) )

    def testBoundContainsDust( self ) :

        script = self.__scene()
        with Gaffer.Context( script.context() ) as c :
            c.setFrame( 15 )
            bound = script["dust"]["out"].bound( "/dust" )
            points = script["dust"]["out"].object( "/dust" )

        self.assertFalse( bound.isEmpty() )
        for p in points["P"].data :
            self.assertTrue( bound.intersects( p ) )
