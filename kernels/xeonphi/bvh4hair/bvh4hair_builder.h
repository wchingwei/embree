// ======================================================================== //
// Copyright 2009-2013 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "bvh4i/bvh4i_builder.h"

#define BVH_NODE_PREALLOC_FACTOR                 1.15f

namespace embree
{

  /*! derived binned-SAH builder supporting hair primitives */  
  class BVH4HairBuilder : public BVH4iBuilder
  {
  public:
  BVH4HairBuilder(BVH4i* bvh, BuildSource* source, void* geometry) : BVH4iBuilder(bvh,source,geometry) 
      {
      }

    virtual void build(size_t threadIndex, size_t threadCount);
    virtual size_t getNumPrimitives();
    virtual void computePrimRefs(const size_t threadIndex, const size_t threadCount);
    virtual void createAccel    (const size_t threadIndex, const size_t threadCount);
    virtual void printBuilderName();

  protected:
    TASK_FUNCTION(BVH4HairBuilder,computePrimRefsBezierCurves);
    TASK_FUNCTION(BVH4HairBuilder,createBezierCurvesAccel);    
  };



}