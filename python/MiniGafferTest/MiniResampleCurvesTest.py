import unittest
import imath
import pathlib
import os

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferOSL

import GafferTest
import GafferSceneTest

import MiniGaffer


examples = os.path.join(pathlib.Path( __file__ ).resolve().parents[2], "examples")

class MiniResampleCurvesTest( GafferSceneTest.SceneTestCase ) :
    def testConstruct( self ) :
        node = MiniGaffer.MiniResampleCurves()
        self.assertEqual( node.getName(), "MiniResampleCurves" )

    def testNonCurveGeometryIsLeftUnchanged( self ):
        sphere_node = GafferScene.Sphere( "Sphere" )
        resample_node  = MiniGaffer.MiniResampleCurves("Resample")
        resample_node["in"].setInput( sphere_node["out"] )
        self.assertEqual(resample_node["out"].object( "/sphere" ), sphere_node["out"].object( "/sphere" ))

    def testCanDoubleTheNumberOfControlPoints(self):
        s = GafferScene.SceneReader()
        s["fileName"].setValue("{}/data/curves.usda".format(examples))
        t = MiniGaffer.MiniResampleCurves()
        t["in"].setInput( s["out"] )

        # check the input is Biezer
        input_object = s["out"].object("/Linear/Ribbons/ConstantWidth")
        self.assertEqual(IECore.CubicBasisf(IECore.StandardCubicBasis.Linear), input_object.basis())
        input_num_control_points = len(input_object["P"].data)
        f = GafferScene.PathFilter()
        f["paths"].setValue( IECore.StringVectorData( [ "/Linear/Ribbons/ConstantWidth" ] ) )
        t["filter"].setInput( f["out"] )

        t["mode"].setValue(1) #fraction
        t["fraction"].setValue(2.0)

        output_object = t["out"].object("/Linear/Ribbons/ConstantWidth")

        self.assertEqual(len(output_object["P"].data), 2 * input_num_control_points)

    def testCanSetTheAbsoluteNumberOfControlPoints(self):
        s = GafferScene.SceneReader()
        s["fileName"].setValue("{}/data/curves.usda".format(examples))
        t = MiniGaffer.MiniResampleCurves()
        t["in"].setInput( s["out"] )

        # check the input is Biezer
        input_object = s["out"].object("/Linear/Ribbons/ConstantWidth")
        self.assertEqual(IECore.CubicBasisf(IECore.StandardCubicBasis.Linear), input_object.basis())
        input_num_control_points = len(input_object["P"].data)
        f = GafferScene.PathFilter()
        f["paths"].setValue( IECore.StringVectorData( [ "/Linear/Ribbons/ConstantWidth" ] ) )
        t["filter"].setInput( f["out"] )

        t["mode"].setValue(0) #fraction
        t["numControlPoints"].setValue(2)

        output_object = t["out"].object("/Linear/Ribbons/ConstantWidth")

        self.assertEqual(len(output_object["P"].data), 2)
