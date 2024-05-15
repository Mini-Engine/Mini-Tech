#pragma once

#include "MiniGafferVDB/Export.h"

#include "Gaffer/Context.h"

#include <openvdb/openvdb.h>

namespace MiniGafferVDB
{

    template<template<typename> class F, typename N, typename R>
    class ScalarGridDispatcher
    {
    public:
        ScalarGridDispatcher(const N* node, const Gaffer::Context* context ) : node( node ), context( context ) {}

        R operator()( openvdb::GridBase::Ptr grid ) const
        {
            if (grid->valueType() == openvdb::typeNameAsString<double>())
            {
                F<openvdb::DoubleGrid> f;
                openvdb::DoubleGrid::Ptr typedGrid = openvdb::GridBase::grid<openvdb::DoubleGrid>(grid);
                return f(typedGrid, node, context );

            } else if (grid->valueType() == openvdb::typeNameAsString<float>())
            {
                F<openvdb::FloatGrid> f;
                openvdb::FloatGrid::Ptr typedGrid = openvdb::GridBase::grid<openvdb::FloatGrid>(grid);
                return f(typedGrid, node, context );
            }

            return R();

        }

        R operator()( openvdb::GridBase::ConstPtr grid ) const
        {
            if ( grid->valueType() == openvdb::typeNameAsString<double>() )
            {
                F<openvdb::DoubleGrid> f;

                openvdb::DoubleGrid::ConstPtr constTypedGrid = openvdb::GridBase::constGrid<openvdb::DoubleGrid>( grid );
                return f( constTypedGrid, node, context );
            }
            else if ( grid->valueType() == openvdb::typeNameAsString<float>() )
            {
                F<openvdb::FloatGrid> f;

                openvdb::FloatGrid::ConstPtr constTypedGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( grid );
                return f( constTypedGrid, node, context );
            }

            return R();

        }

        const N* node;
        const Gaffer::Context* context;
    };


}




