import unittest
import imath
import pathlib
import os

import IECore
import IECoreScene

import Gaffer
import GafferScene

import GafferTest
import GafferSceneTest

import MiniGaffer

examples = os.path.join(pathlib.Path( __file__ ).resolve().parents[2], "examples")

class MiniCurvesTypeTest( GafferSceneTest.SceneTestCase ) :

    def testConstruct( self ) :

        node = MiniGaffer.MiniCurvesType()
        self.assertEqual( node.getName(), "MiniCurvesType" )

    def testNonCurveGeometryIsLeftUnchanged( self ):
        sphere_node = GafferScene.Sphere( "Sphere" )
        curves_type_node  = MiniGaffer.MiniCurvesType("CurvesType")
        curves_type_node["in"].setInput( sphere_node["out"] )
        self.assertEqual(curves_type_node["out"].object( "/sphere" ), sphere_node["out"].object( "/sphere" ))
    def testCanChangeTopologyOfCurves(self):
        s = GafferScene.SceneReader()
        s["fileName"].setValue("{}/data/curves.usda".format(examples))
        t = MiniGaffer.MiniCurvesType()
        t["in"].setInput( s["out"] )

        # check the input is Biezer
        self.assertEqual(IECore.CubicBasisf(IECore.StandardCubicBasis.Bezier), s["out"].object("/Cubic/Ribbons/ConstantWidth").basis())

        f = GafferScene.PathFilter()
        f["paths"].setValue( IECore.StringVectorData( [ "/Cubic/Ribbons/ConstantWidth" ] ) )
        t["filter"].setInput( f["out"] )

        t["basis"].setValue(0) # linear basis

        self.assertEqual(IECore.CubicBasisf(IECore.StandardCubicBasis.Linear), t["out"].object("/Cubic/Ribbons/ConstantWidth").basis())
