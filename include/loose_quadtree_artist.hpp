#ifndef ALH_LOOSE_QUADTREE_ARTIST_HPP
#define ALH_LOOSE_QUADTREE_ARTIST_HPP

#include "raylib.h"

#define private public
#include "loose_quadtree.hpp"
#undef private

namespace alh {

template<typename T=void*, uint64_t MAX_DEPTH=4>
struct loose_quadtree_artist_t {

    using id_t = typename loose_quadtree_t<T, MAX_DEPTH>::id_t;
    using aabb_t = typename loose_quadtree_t<T, MAX_DEPTH>::aabb_t;

    static constexpr id_t empty = loose_quadtree_t<T, MAX_DEPTH>::empty;

    loose_quadtree_artist_t(loose_quadtree_t<T, MAX_DEPTH> &tree) : tree(tree) {}

    void draw() {
        draw_recursive(tree.root, tree.aabb);
    }

    void draw_query(aabb_t query_bb) {
        draw_query_recursive(tree.root, query_bb);
    }

private:
    loose_quadtree_artist_t() {}

    void draw_recursive(id_t nid, aabb_t bb) {
        DrawRectangleLines(bb.min.x,
                           bb.min.y,
                           bb.max.x - bb.min.x,
                           bb.max.y - bb.min.y,
                           {130, 130, 130, 255});
        aabb_t bb_nw, bb_ne, bb_sw, bb_se;
        tree.split_4(bb, bb_nw, bb_ne, bb_sw, bb_se);

        bool is_not_leaf = false;
        if (empty != tree.nodes[nid].nw && (is_not_leaf=true)) draw_recursive(tree.nodes[nid].nw, bb_nw);
        if (empty != tree.nodes[nid].ne && (is_not_leaf=true)) draw_recursive(tree.nodes[nid].ne, bb_ne);
        if (empty != tree.nodes[nid].sw && (is_not_leaf=true)) draw_recursive(tree.nodes[nid].sw, bb_sw);
        if (empty != tree.nodes[nid].se && (is_not_leaf=true)) draw_recursive(tree.nodes[nid].se, bb_se);

        if (!is_not_leaf) {
            auto start_inc = tree.node_points_begin[nid];
            auto end_excl = tree.node_points_begin[nid + 1];
            for (auto i = start_inc; i < end_excl; i++) {
                Vector2 min = {tree.boxes[i].aabb.min.x, tree.boxes[i].aabb.min.y};
                Vector2 max = {tree.boxes[i].aabb.max.x, tree.boxes[i].aabb.max.y};
                DrawRectangleLines(min.x,
                                   min.y,
                                   max.x - min.x,
                                   max.y - min.y,
                                   {200, 200, 200, 255});
            }
        }
    }

    void draw_query_recursive(id_t nid, aabb_t query_bb) {
        if (query_bb.intersect(tree.node_bbs[nid])) {
            if (empty != tree.nodes[nid].nw) draw_query_recursive(tree.nodes[nid].nw, query_bb);
            if (empty != tree.nodes[nid].ne) draw_query_recursive(tree.nodes[nid].ne, query_bb);
            if (empty != tree.nodes[nid].sw) draw_query_recursive(tree.nodes[nid].sw, query_bb);
            if (empty != tree.nodes[nid].se) draw_query_recursive(tree.nodes[nid].se, query_bb);

            auto bb = tree.node_bbs[nid];
            DrawRectangleLines(bb.min.x,
                               bb.min.y,
                               bb.max.x - bb.min.x,
                               bb.max.y - bb.min.y,
                               {255, 0, 0, 255});
        }
    }

    loose_quadtree_t<T, MAX_DEPTH> &tree;
};

};

#endif