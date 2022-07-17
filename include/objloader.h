#ifndef OBJLOADER_H
#define OBJLOADER_H
#define TINYOBJLOADER_IMPLEMENTATION

#include <vector>

bool loadObjModel(std::string path, std::vector<float>& buffers);
bool loadObjModel(std::string path, std::vector<float>& verts, std::vector<float>& normals, bool normalize = false);

#endif // OBJLOADER_H
