import os
import pathlib

import imath

import IECore
import IECoreVDB

import Gaffer
import GafferScene
import GafferSceneTest

import MiniGaffer
import MiniGafferVDB

class DustHitVolumeTest( GafferSceneTest.SceneTestCase ) :

    def __reference( self, script ) :

        for path in os.environ.get( "GAFFER_REFERENCE_PATHS", "" ).split( os.pathsep ) :
            candidate = pathlib.Path( path ) / "dustHitVolume.grf"
            if candidate.exists() :
                script["volume"] = Gaffer.Reference()
                script["volume"].load( str( candidate ) )
                return script["volume"]

        self.skipTest( "dustHitVolume.grf not found in GAFFER_REFERENCE_PATHS" )

    def testDustBecomesVolume( self ) :

        script = Gaffer.ScriptNode()

        # A cube falling onto a floor, making dust when it lands on frame 10.
        script["floor"] = GafferScene.Plane()
        script["floor"]["name"].setValue( "floor" )
        script["floor"]["dimensions"].setValue( imath.V2f( 10 ) )
        script["floor"]["transform"]["rotate"]["x"].setValue( -90 )

        script["cube"] = GafferScene.Cube()
        script["cube"]["name"].setValue( "cube" )
        script["expression"] = Gaffer.Expression()
        script["expression"].setExpression(
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

        volume = self.__reference( script )
        volume["points"].setInput( script["dust"]["out"] )

        with Gaffer.Context( script.context() ) as c :

            # No dust yet, so an empty grid.
            c.setFrame( 5 )
            self.assertEqual( volume["out"].childNames( "/" ), IECore.InternedStringVectorData( [ "dustVolume" ] ) )
            emptyHash = volume["out"].objectHash( "/dustVolume" )

            c.setFrame( 14 )
            vdb = volume["out"].object( "/dustVolume" )
            self.assertIsInstance( vdb, IECoreVDB.VDBObject )
            self.assertEqual( vdb.gridNames(), [ "density" ] )

            # Once there is dust the volume has to be recomputed.
            self.assertNotEqual( volume["out"].objectHash( "/dustVolume" ), emptyHash )
