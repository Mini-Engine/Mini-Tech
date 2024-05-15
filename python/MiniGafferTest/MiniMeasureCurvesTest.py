import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferOSL

import GafferTest
import GafferSceneTest
import os
import pathlib
import MiniGaffer

examples = os.path.join(pathlib.Path( __file__ ).resolve().parents[2], "examples")

class MiniMeasureCurvesTest( GafferSceneTest.SceneTestCase ) :
    def testConstruct( self ) :
        node = MiniGaffer.MiniMeasureCurves()
        self.assertEqual( node.getName(), "MiniMeasureCurves" )


    def testCompute( self ):

        s = GafferScene.SceneReader()
        s["fileName"].setValue("{}/data/curves.usda".format(examples))
        m = MiniGaffer.MiniMeasureCurves()
        m["in"].setInput( s["out"] )

        f = GafferScene.PathFilter()
        f["paths"].setValue( IECore.StringVectorData( [ "/Cubic/Ribbons/ConstantWidth" ] ) )
        m["filter"].setInput( f["out"] )

        curves = m["out"].object( "/Cubic/Ribbons/ConstantWidth" )
        self.assertIn( "length", curves )

        self.assertIsInstance( curves["length"].data, IECore.FloatVectorData )

    def testLengthNamePlug( self ):

        s = GafferScene.SceneReader()
        s["fileName"].setValue("{}/data/curves.usda".format(examples))
        m = MiniGaffer.MiniMeasureCurves()
        m["in"].setInput( s["out"] )

        f = GafferScene.PathFilter()
        f["paths"].setValue( IECore.StringVectorData( [ "/Cubic/Ribbons/ConstantWidth" ] ) )
        m["filter"].setInput( f["out"] )
        m["lengthPrimvar"].setValue("testo")
        curves = m["out"].object( "/Cubic/Ribbons/ConstantWidth" )
        self.assertIn( "testo", curves )

        self.assertIsInstance( curves["testo"].data, IECore.FloatVectorData )

    def testFilterAllPaths( self ):

        s = GafferScene.SceneReader()
        s["fileName"].setValue("{}/data/curves.usda".format(examples))
        m = MiniGaffer.MiniMeasureCurves()
        m["in"].setInput( s["out"] )

        f = GafferScene.PathFilter()
        f["paths"].setValue( IECore.StringVectorData( [ "/..." ] ) )
        m["filter"].setInput( f["out"] )

        curves = m["out"].object( "/Cubic/Ribbons/ConstantWidth" )
        self.assertIn( "length", curves )
        self.assertIsInstance( curves["length"].data, IECore.FloatVectorData )


    def testCanCalculateLengthCorrectly(self):
        pass
