#include <string.h>
#include <stdlib.h>

#include <3dmr/material/solid.h>
#include <3dmr/render/vertex_array.h>
#include <3dmr/mesh/box.h>

#include "render_octree.h"

static struct RenderOctree {
    struct Mesh box;
    struct VertexArray arr;
    struct SolidMaterialParams* params;
    struct Material* mat;
} renderOctree;

static void set_color(struct Material* mat, float r, float g, float b) {
    struct SolidMaterialParams* params = mat->params;
    material_param_set_vec3_elems(&params->color, r, g, b);
    material_use(mat);
}

static void render_phys_object(struct PhysObject* obj, struct Material* mat, struct VertexArray* arr) {
    Mat4 model;
    Mat3 invNormal;
    load_id4(model);
    load_id3(invNormal);
    scale3v(model[0], 2 * obj->aabbDimensions[0]);
    scale3v(model[1], 2 * obj->aabbDimensions[1]);
    scale3v(model[2], 2 * obj->aabbDimensions[2]);
    memcpy(model[3], obj->pos, sizeof(Vec3));
    set_color(mat, 1, 0.2, 0);
    material_set_matrices(mat, model, invNormal);
    vertex_array_render(arr);
}

static void render_cell(struct PhysOctreeCell* cell, struct Material* mat, struct VertexArray* arr) {
    unsigned int i;
    Mat4 model;
    Mat3 invNormal;
    if (!cell) return;
    load_id4(model);
    load_id3(invNormal);
    scale3v(model[0], 2 * cell->radius);
    scale3v(model[1], 2 * cell->radius);
    scale3v(model[2], 2 * cell->radius);
    memcpy(model[3], cell->pos, sizeof(Vec3));
    set_color(mat, 0, 1, 0.2);
    material_set_matrices(mat, model, invNormal);
    vertex_array_render(arr);
    if (cell->objects) {
        struct List* cur = cell->objects;
        while (cur) {
            render_phys_object(cur->obj, mat, arr);
            cur = cur->tail;
        }
    }
    for (i = 0; i < 8; i++) {
        render_cell(cell->children[i], mat, arr);
    }
}

void render_octree(struct PhysOctree* octree) {
    render_cell(octree->root, renderOctree.mat, &renderOctree.arr);
}

void render_octree_init() {
    make_box(&renderOctree.box, 1., 1., 1.);
    vertex_array_gen(&renderOctree.box, &renderOctree.arr);
    renderOctree.params = solid_material_params_new();
    renderOctree.mat = solid_material_new(renderOctree.box.flags, renderOctree.params);
    renderOctree.mat->polygonMode = GL_LINE;
}

void render_octree_cleanup() {
    free(renderOctree.params);
    free(renderOctree.mat);
    mesh_free(&renderOctree.box);
    vertex_array_del(&renderOctree.arr);
}
