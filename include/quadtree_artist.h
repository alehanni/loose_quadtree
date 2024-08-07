#ifndef QUADTREE_ARTIST_H
#define QUADTREE_ARTIST_H

#include "quadtree.h"
#include "utiltypes.h"
#include "renderer.h"

// a class that draws a quadtree

struct quadtree_artist {

    quadtree_artist(quadtree &qt) : _qt(qt)
    { }

    void draw() {
        box_t big_box = point_hull(&_qt.pointboxes.front(), &_qt.pointboxes.back() + 1);
        draw_helper(_qt.root, big_box);
    }

    void draw_query(box_t query_bb) {
        if (query_bb.intersect(_qt.node_bbs[_qt.root]))
            draw_query_helper(_qt.root, query_bb);
    }

protected:

    void draw_helper(node_id nid, box_t const& bb) {
        auto r = entt::locator<renderer>::value();

        r->draw_rectangle(
            (vec2_t){bb.min.x, bb.min.y},
            (vec2_t){bb.max.x, bb.max.y},
            (rgba_t){130, 130, 130, 255});

        // split recursively if there are children, draw points otherwise
        box_t bb_nw, bb_ne, bb_sw, bb_se;
        split_4(bb, bb_nw, bb_ne, bb_sw, bb_se);

        bool is_not_leaf = false;
        if (empty != _qt.nodes[nid].nw && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].nw, bb_nw);
        if (empty != _qt.nodes[nid].ne && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].ne, bb_ne);
        if (empty != _qt.nodes[nid].sw && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].sw, bb_sw);
        if (empty != _qt.nodes[nid].se && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].se, bb_se);

        if (!is_not_leaf){
            auto start_inc = _qt.node_points_begin[nid];
            auto end_excl = _qt.node_points_begin[nid + 1];
            for (auto i = start_inc; i < end_excl; i++) {
                r->draw_rectangle(
                    (vec2_t){_qt.pointboxes[i].min.x, _qt.pointboxes[i].min.y},
                    (vec2_t){_qt.pointboxes[i].max.x, _qt.pointboxes[i].max.y},
                    (rgba_t){200, 200, 200, 255});
            }
        }
    }

    void draw_query_helper(node_id nid, box_t &query_bb) {
        auto r = entt::locator<renderer>::value();

        if (query_bb.intersect(_qt.node_bbs[nid])) {

            if (empty != _qt.nodes[nid].nw) draw_query_helper(_qt.nodes[nid].nw, query_bb);
            if (empty != _qt.nodes[nid].ne) draw_query_helper(_qt.nodes[nid].ne, query_bb);
            if (empty != _qt.nodes[nid].sw) draw_query_helper(_qt.nodes[nid].sw, query_bb);
            if (empty != _qt.nodes[nid].se) draw_query_helper(_qt.nodes[nid].se, query_bb);

            r->draw_rectangle(
                (vec2_t){_qt.node_bbs[nid].min.x, _qt.node_bbs[nid].min.y},
                (vec2_t){_qt.node_bbs[nid].max.x, _qt.node_bbs[nid].max.y},
                (rgba_t){255, 0, 0, 255});
        }
    }

    quadtree &_qt;
};

#endif