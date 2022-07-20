#include <iostream>
#include <algorithm>

#include "objloader.h"

#include "tiny_obj_loader.h"
#include <glm/glm.hpp>

struct vec3
{
  float v[3];
  vec3()
  {
    v[0] = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
  }
};

static void computeSmoothingShape(tinyobj::attrib_t& inattrib, tinyobj::shape_t& inshape,
                                  std::vector<std::pair<unsigned int, unsigned int>>& sortedids,
                                  unsigned int idbegin, unsigned int idend,
                                  std::vector<tinyobj::shape_t>& outshapes,
                                  tinyobj::attrib_t& outattrib)
{
    unsigned int sgroupid = sortedids[idbegin].first;
    bool hasmaterials = inshape.mesh.material_ids.size();
    // Make a new shape from the set of faces in the range [idbegin, idend).
    outshapes.emplace_back();
    tinyobj::shape_t& outshape = outshapes.back();
    outshape.name = inshape.name;
    // Skip lines and points.
    
    std::unordered_map<unsigned int, unsigned int> remap;
    for (unsigned int id = idbegin; id < idend; ++id)
    {
        unsigned int face = sortedids[id].second;
        outshape.mesh.num_face_vertices.push_back(3); // always triangles
        if (hasmaterials)
            outshape.mesh.material_ids.push_back(inshape.mesh.material_ids[face]);
        
        outshape.mesh.smoothing_group_ids.push_back(sgroupid);
        // Skip tags.

        for (unsigned int v = 0; v < 3; ++v)
        {
            tinyobj::index_t inidx = inshape.mesh.indices[3*face + v], outidx;
            assert(inidx.vertex_index != -1);
            auto iter = remap.find(inidx.vertex_index);
            // Smooth group 0 disables smoothing so no shared vertices in that case.
            if (sgroupid && iter != remap.end())
            {
                outidx.vertex_index = (*iter).second;
                outidx.normal_index = outidx.vertex_index;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : outidx.vertex_index;
            }
            else
            {
                assert(outattrib.vertices.size() % 3 == 0);
                unsigned int offset = static_cast<unsigned int>(outattrib.vertices.size() / 3);
                outidx.vertex_index = outidx.normal_index = offset;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : offset;
                outattrib.vertices.push_back(inattrib.vertices[3*inidx.vertex_index  ]);
                outattrib.vertices.push_back(inattrib.vertices[3*inidx.vertex_index+1]);
                outattrib.vertices.push_back(inattrib.vertices[3*inidx.vertex_index+2]);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                if (inidx.texcoord_index != -1)
                {
                    outattrib.texcoords.push_back(inattrib.texcoords[2*inidx.texcoord_index  ]);
                    outattrib.texcoords.push_back(inattrib.texcoords[2*inidx.texcoord_index+1]);
                }
                remap[inidx.vertex_index] = offset;
            }
            outshape.mesh.indices.push_back(outidx);
        }
    }
}

static void computeSmoothingShapes(tinyobj::attrib_t &inattrib,
                                   std::vector<tinyobj::shape_t>& inshapes,
                                   std::vector<tinyobj::shape_t>& outshapes,
                                   tinyobj::attrib_t& outattrib)
{
    for (size_t s = 0, slen = inshapes.size() ; s < slen; ++s)
    {
        tinyobj::shape_t& inshape = inshapes[s];
        unsigned int numfaces = static_cast<unsigned int>(inshape.mesh.smoothing_group_ids.size());
        assert(numfaces);
        std::vector<std::pair<unsigned int,unsigned int>> sortedids(numfaces);
        for (unsigned int i = 0; i < numfaces; ++i)
            sortedids[i] = std::make_pair(inshape.mesh.smoothing_group_ids[i], i);
        sort(sortedids.begin(), sortedids.end());

        unsigned int activeid = sortedids[0].first;
        unsigned int id = activeid, idbegin = 0, idend = 0;
        // Faces are now bundled by smoothing group id, create shapes from these.
        while (idbegin < numfaces)
        {
            while (activeid == id && ++idend < numfaces)
            id = sortedids[idend].first;
            computeSmoothingShape(inattrib, inshape, sortedids, idbegin, idend, outshapes, outattrib);
            activeid = id;
            idbegin = idend;
        }
  }
}

static void computeAllSmoothingNormals(tinyobj::attrib_t& attrib,
                                       std::vector<tinyobj::shape_t>& shapes)
{
    vec3 p[3];
    for (size_t s = 0, slen = shapes.size(); s < slen; ++s)
    {
        const tinyobj::shape_t& shape(shapes[s]);
        size_t facecount = shape.mesh.num_face_vertices.size();
        assert(shape.mesh.smoothing_group_ids.size());
        
        for (size_t f = 0, flen = facecount; f < flen; ++f)
        {
            for (unsigned int v = 0; v < 3; ++v)
            {
                tinyobj::index_t idx = shape.mesh.indices[3*f + v];
                assert(idx.vertex_index != -1);
                p[v].v[0] = attrib.vertices[3*idx.vertex_index  ];
                p[v].v[1] = attrib.vertices[3*idx.vertex_index+1];
                p[v].v[2] = attrib.vertices[3*idx.vertex_index+2];
            }
            
            // cross(p[1] - p[0], p[2] - p[0])
            float nx = (p[1].v[1] - p[0].v[1]) * (p[2].v[2] - p[0].v[2]) - (p[1].v[2] - p[0].v[2]) * (p[2].v[1] - p[0].v[1]);
            float ny = (p[1].v[2] - p[0].v[2]) * (p[2].v[0] - p[0].v[0]) - (p[1].v[0] - p[0].v[0]) * (p[2].v[2] - p[0].v[2]);
            float nz = (p[1].v[0] - p[0].v[0]) * (p[2].v[1] - p[0].v[1]) - (p[1].v[1] - p[0].v[1]) * (p[2].v[0] - p[0].v[0]);
            
            // Don't normalize here.
            for (unsigned int v = 0; v < 3; ++v)
            {
                tinyobj::index_t idx = shape.mesh.indices[3*f + v];
                attrib.normals[3*idx.normal_index  ] += nx;
                attrib.normals[3*idx.normal_index+1] += ny;
                attrib.normals[3*idx.normal_index+2] += nz;
            }
        }
    }
    
    assert(attrib.normals.size() % 3 == 0);
    for (size_t i = 0, nlen = attrib.normals.size() / 3; i < nlen; ++i)
    {
        tinyobj::real_t& nx = attrib.normals[3*i  ];
        tinyobj::real_t& ny = attrib.normals[3*i+1];
        tinyobj::real_t& nz = attrib.normals[3*i+2];
        tinyobj::real_t len = sqrtf(nx*nx + ny*ny + nz*nz);
        tinyobj::real_t scale = len == 0 ? 0 : 1 / len;
        nx *= scale;
        ny *= scale;
        nz *= scale;
    }
}

bool loadObjModel(std::string path, std::vector<float> &verts, std::vector<float> &norms, bool normalize)
{
    tinyobj::attrib_t inattrib;
    std::vector<tinyobj::shape_t> inshapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn,err;

    if (!tinyobj::LoadObj(&inattrib, &inshapes, &materials, &warn, &err, path.c_str()))
    {
        std::cout << "error in open objModel :" << path << std::endl;
        return false;
    }

    bool regenNormals = inattrib.normals.size() == 0;
    tinyobj::attrib_t outattrib;
    std::vector<tinyobj::shape_t> outshapes;
    if(regenNormals)
    {
        computeSmoothingShapes(inattrib, inshapes, outshapes, outattrib);
        computeAllSmoothingNormals(outattrib, outshapes);
    }

    std::vector<tinyobj::shape_t>& shapes = regenNormals ? outshapes : inshapes;
    tinyobj::attrib_t& attrib = regenNormals ? outattrib : inattrib;

    float bmin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    float bmax[3] = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    for (const auto & shape : shapes)
    {
        for(size_t f = 0; f < shape.mesh.indices.size() / 3; ++f)
        {
            auto id = shape.mesh.indices[f];
            tinyobj::index_t id0 = shape.mesh.indices[3 * f + 0];
            tinyobj::index_t id1 = shape.mesh.indices[3 * f + 1];
            tinyobj::index_t id2 = shape.mesh.indices[3 * f + 2];

            float v[3][3];
            float n[3][3];

            for(size_t k = 0; k < 3; ++k)
            {
                int f0 = id0.vertex_index;
                int f1 = id1.vertex_index;
                int f2 = id2.vertex_index;
                assert(f0 >= 0);
                assert(f1 >= 0);
                assert(f2 >= 0);

                v[0][k] = attrib.vertices[3 * f0 + k];
                v[1][k] = attrib.vertices[3 * f1 + k];
                v[2][k] = attrib.vertices[3 * f2 + k];
                bmin[k] = std::min(v[0][k], bmin[k]);
                bmin[k] = std::min(v[1][k], bmin[k]);
                bmin[k] = std::min(v[2][k], bmin[k]);
                bmax[k] = std::max(v[0][k], bmax[k]);
                bmax[k] = std::max(v[1][k], bmax[k]);
                bmax[k] = std::max(v[2][k], bmax[k]);

                int n0 = id0.normal_index;
                int n1 = id1.normal_index;
                int n2 = id2.normal_index;
                assert(size_t(3 * n0 + k) < attrib.normals.size());
                assert(size_t(3 * n1 + k) < attrib.normals.size());
                assert(size_t(3 * n2 + k) < attrib.normals.size());
                n[0][k] = attrib.normals[3 * n0 + k];
                n[1][k] = attrib.normals[3 * n1 + k];
                n[2][k] = attrib.normals[3 * n2 + k];
            }

            for(int i = 0; i < 3; ++i)
            {
                verts.push_back(v[i][0]);
                verts.push_back(v[i][1]);
                verts.push_back(v[i][2]);

                norms.push_back(n[i][0]);
                norms.push_back(n[i][1]);
                norms.push_back(n[i][2]);
            }
        }
    }

    if(normalize)
    {
        for(auto & v : verts)
            v /= std::max(bmax[0], std::max(bmax[1], bmax[2]));
    }

    return true;
}

bool loadObjModel(std::string path, std::vector<float>& buffers)
{
    tinyobj::attrib_t inattrib;
    std::vector<tinyobj::shape_t> inshapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn,err;

    if (!tinyobj::LoadObj(&inattrib, &inshapes, &materials, &warn, &err, path.c_str()))
    {
        std::cout << "error in open objModel :" << path << std::endl;
        return false;
    }

    bool regenNormals = inattrib.normals.size() == 0;
    tinyobj::attrib_t outattrib;
    std::vector<tinyobj::shape_t> outshapes;
    if(regenNormals)
    {
        computeSmoothingShapes(inattrib, inshapes, outshapes, outattrib);
        computeAllSmoothingNormals(outattrib, outshapes);
    }

    std::vector<tinyobj::shape_t>& shapes = regenNormals ? outshapes : inshapes;
    tinyobj::attrib_t& attrib = regenNormals ? outattrib : inattrib;

    float bmin[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    float bmax[3] = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    for (const auto & shape : shapes)
    {
        for(size_t f = 0; f < shape.mesh.indices.size() / 3; ++f)
        {
            auto id = shape.mesh.indices[f];
            tinyobj::index_t id0 = shape.mesh.indices[3 * f + 0];
            tinyobj::index_t id1 = shape.mesh.indices[3 * f + 1];
            tinyobj::index_t id2 = shape.mesh.indices[3 * f + 2];

            float v[3][3];
            float n[3][3];

            for(size_t k = 0; k < 3; ++k)
            {
                int f0 = id0.vertex_index;
                int f1 = id1.vertex_index;
                int f2 = id2.vertex_index;
                assert(f0 >= 0);
                assert(f1 >= 0);
                assert(f2 >= 0);

                v[0][k] = attrib.vertices[3 * f0 + k];
                v[1][k] = attrib.vertices[3 * f1 + k];
                v[2][k] = attrib.vertices[3 * f2 + k];
                bmin[k] = std::min(v[0][k], bmin[k]);
                bmin[k] = std::min(v[1][k], bmin[k]);
                bmin[k] = std::min(v[2][k], bmin[k]);
                bmax[k] = std::max(v[0][k], bmax[k]);
                bmax[k] = std::max(v[1][k], bmax[k]);
                bmax[k] = std::max(v[2][k], bmax[k]);

                int n0 = id0.normal_index;
                int n1 = id1.normal_index;
                int n2 = id2.normal_index;
                assert(size_t(3 * n0 + k) < attrib.normals.size());
                assert(size_t(3 * n1 + k) < attrib.normals.size());
                assert(size_t(3 * n2 + k) < attrib.normals.size());
                n[0][k] = attrib.normals[3 * n0 + k];
                n[1][k] = attrib.normals[3 * n1 + k];
                n[2][k] = attrib.normals[3 * n2 + k];
            }

            for(int i = 0; i < 3; ++i)
            {
                buffers.push_back(v[i][0]);
                buffers.push_back(v[i][1]);
                buffers.push_back(v[i][2]);

                buffers.push_back(n[i][0]);
                buffers.push_back(n[i][1]);
                buffers.push_back(n[i][2]);
            }
        }
    }

    // if(normalize)
    // {
    //     for(auto & v : verts)
    //         v /= std::max(bmax[0], std::max(bmax[1], bmax[2]));
    // }

    return true;
}