import IECore
import Gaffer
import pathlib
import os

examples = os.path.join(pathlib.Path( __file__ ).resolve().parents[2], "examples")


def __scriptAdded( container, script ) :

    variables = script["variables"]

    if "miniExamplesRoot" not in variables :
        examplesRoot = variables.addMember(
            "mini:examplesRoot",
            IECore.StringData( examples ),
            "miniExamplesRoot"
        )

    Gaffer.MetadataAlgo.setReadOnly( variables["miniExamplesRoot"]["name"], True )

application.root()["scripts"].childAddedSignal().connect( __scriptAdded, scoped = False )