// ======================================================================== //
// Copyright 2009-2014 Intel Corporation                                    //
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

#include "primitive.h"
#include "common/scene_subdivision.h"

#define SUBDIVISION_LEVEL 0

namespace embree
{
  struct SubdivPatch1
  {
    struct Type : public PrimitiveType 
    {
      Type ();
      size_t blocks(size_t x) const; 
      size_t size(const char* This) const;
    };

    static Type type;

  public:

    enum {
      REGULAR_PATCH = 1,
      HAS_BORDERS   = 2,
      HAS_CREASES   = 4
    };

    /*! Default constructor. */
    __forceinline SubdivPatch1 () {}

    /*! Construction from vertices and IDs. */
    __forceinline SubdivPatch1 (const SubdivMesh::HalfEdge* edge, 
				const Vec3fa* vertices, 
                                const unsigned int geom, 
				const unsigned int prim, 
				const unsigned int subdivision_level,
				const bool last)
      : first_half_edge(edge), vertices(vertices), geom(geom), prim(prim | (last << 31)), subdivision_level(subdivision_level)
    {
      flags = 0;
      if (first_half_edge->isFaceRegular()) 
	flags |= REGULAR_PATCH;      
    }

    __forceinline bool isRegular() const
    {
      return (flags & REGULAR_PATCH) == REGULAR_PATCH;
    }


    /*! returns required number of primitive blocks for N primitives */
    static __forceinline size_t blocks(size_t N) { return N; }

    /*! return geometry ID */
    template<bool list>
    __forceinline unsigned int geomID() const { 
      return geom; 
    }

    /*! return primitive ID */
    template<bool list>
    __forceinline unsigned int primID() const { 
      if (list) return prim & 0x7FFFFFFF; 
      else      return prim; 
    }

    /*! checks if this is the last primitive in list leaf mode */
    __forceinline int last() const { 
      return prim & 0x80000000; 
    }

    /*! builder interface to fill primitive */
    __forceinline void fill(atomic_set<PrimRefBlock>::block_iterator_unsafe& prims, Scene* scene, const bool list)
    {
      const PrimRef& prim = *prims;
      prims++;

      const unsigned int last   = list && !prims;
      const unsigned int geomID = prim.geomID();
      const unsigned int primID = prim.primID();
      const SubdivMesh* const subdiv_mesh = scene->getSubdivMesh(geomID);
      new (this) SubdivPatch1(&subdiv_mesh->getHalfEdgeForQuad( primID ),
			      subdiv_mesh->getVertexPositionPtr(),
			      geomID,
			      primID,
			      SUBDIVISION_LEVEL,
			      last); 
    }

    /*! builder interface to fill primitive */
    __forceinline void fill(const PrimRef* prims, size_t& i, size_t end, Scene* scene, const bool list)
    {
      const PrimRef& prim = prims[i];
      i++;

      const unsigned int last = list && i >= end;
      const unsigned int geomID = prim.geomID();
      const unsigned int primID = prim.primID();
      const SubdivMesh* const subdiv_mesh = scene->getSubdivMesh(geomID);
      new (this) SubdivPatch1(&subdiv_mesh->getHalfEdgeForQuad( primID ),
			      subdiv_mesh->getVertexPositionPtr(),
			      geomID,
			      primID,
			      SUBDIVISION_LEVEL,
			      last); 
    }

    __forceinline void init( IrregularCatmullClarkPatch& patch) const
    {
      for (size_t i=0;i<4;i++)
	patch.ring[i].init(first_half_edge + i,vertices);
      patch.geomID = geom;
      patch.primID = prim;
    }

    __forceinline void init( RegularCatmullClarkPatch& cc_patch) const
    {
      // quad(0,0)
      const SubdivMesh::HalfEdge *e11 = first_half_edge->half_circle();
      const SubdivMesh::HalfEdge *e10 = e11->next();
      const SubdivMesh::HalfEdge *e00 = e10->next();
      const SubdivMesh::HalfEdge *e01 = e10->next();

      cc_patch.v[1][1] = vertices[e11->getStartVertexIndex()];
      cc_patch.v[1][0] = vertices[e10->getStartVertexIndex()];
      cc_patch.v[0][0] = vertices[e00->getStartVertexIndex()];
      cc_patch.v[0][1] = vertices[e01->getStartVertexIndex()];

      // quad(0,2)
      const SubdivMesh::HalfEdge *e12 = first_half_edge->opposite()->half_circle();
      const SubdivMesh::HalfEdge *e13 = e12->next();
      const SubdivMesh::HalfEdge *e03 = e13->next();
      const SubdivMesh::HalfEdge *e02 = e03->next();

      cc_patch.v[1][2] = vertices[e12->getStartVertexIndex()];
      cc_patch.v[1][3] = vertices[e13->getStartVertexIndex()];
      cc_patch.v[0][3] = vertices[e03->getStartVertexIndex()];
      cc_patch.v[0][2] = vertices[e02->getStartVertexIndex()];

      // quad(2,0)
      const SubdivMesh::HalfEdge *e21 = first_half_edge->prev()->half_circle();
      const SubdivMesh::HalfEdge *e31 = e21->next();
      const SubdivMesh::HalfEdge *e30 = e31->next();
      const SubdivMesh::HalfEdge *e20 = e30->next();

      cc_patch.v[2][1] = vertices[e21->getStartVertexIndex()];
      cc_patch.v[3][1] = vertices[e31->getStartVertexIndex()];
      cc_patch.v[3][0] = vertices[e30->getStartVertexIndex()];
      cc_patch.v[2][0] = vertices[e20->getStartVertexIndex()];

      // quad(2,2)
      const SubdivMesh::HalfEdge *e22 = first_half_edge->next()->opposite()->half_circle();
      const SubdivMesh::HalfEdge *e32 = e22->next();
      const SubdivMesh::HalfEdge *e33 = e32->next();
      const SubdivMesh::HalfEdge *e23 = e33->next();

      cc_patch.v[2][2] = vertices[e22->getStartVertexIndex()];
      cc_patch.v[3][2] = vertices[e32->getStartVertexIndex()];
      cc_patch.v[3][3] = vertices[e33->getStartVertexIndex()];
      cc_patch.v[2][3] = vertices[e23->getStartVertexIndex()];      
    }
    
  public:
    const SubdivMesh::HalfEdge* first_half_edge;  //!< pointer to first half edge of this patch
    const Vec3fa* vertices;                       //!< pointer to vertex array
    unsigned int subdivision_level;
    unsigned int flags;
    unsigned int geom;                            //!< geometry ID of the subdivision mesh this patch belongs to
    unsigned int prim;                            //!< primitive ID of this subdivision patch
  };
}