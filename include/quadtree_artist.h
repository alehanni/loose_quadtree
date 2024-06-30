#ifndef QUADTREE_ARTIST_H
#define QUADTREE_ARTIST_H

#include "quadtree.h"
#include "utiltypes.h"

// a class that draws a quadtree

template<typename Renderer>
struct quadtree_artist {

    quadtree_artist(quadtree &qt, Renderer &r) : _qt(qt), _r(r)
    { }

    void draw() {
        bbox big_box = bbox();
        std::for_each(
            &_qt.pointboxes.front(),
            &_qt.pointboxes.back(),
            [&big_box](pointbox &pb) mutable {big_box |= pb.mid;}
        );
        draw_helper(_qt.root, big_box);
    }

    void draw_query(bbox query_bb) {
        if (query_bb.intersect(_qt.node_bbs[_qt.root]))
            draw_query_helper(_qt.root, query_bb);
    }

protected:

    void draw_helper(node_id nid, bbox const& bb) {
        _r.draw_rectangle(
            (vec2_t){bb.minx, bb.miny},
            (vec2_t){bb.maxx, bb.maxy},
            (rgba_t){130, 130, 130, 255});

        // split recursively if there are children, draw points otherwise
        bbox bb_nw, bb_ne, bb_sw, bb_se;
        bb.split_4(bb_nw, bb_ne, bb_sw, bb_se);

        bool is_not_leaf = false;
        if (empty != _qt.nodes[nid].nw && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].nw, bb_nw);
        if (empty != _qt.nodes[nid].ne && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].ne, bb_ne);
        if (empty != _qt.nodes[nid].sw && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].sw, bb_sw);
        if (empty != _qt.nodes[nid].se && (is_not_leaf = true)) draw_helper(_qt.nodes[nid].se, bb_se);

        if (!is_not_leaf){
            auto start_inc = _qt.node_points_begin[nid];
            auto end_excl = _qt.node_points_begin[nid + 1];
            for (auto i = start_inc; i < end_excl; i++) {
                _r.draw_rectangle(
                    (vec2_t){_qt.pointboxes[i].bb.minx, _qt.pointboxes[i].bb.miny},
                    (vec2_t){_qt.pointboxes[i].bb.maxx, _qt.pointboxes[i].bb.maxy},
                    (rgba_t){200, 200, 200, 255});
            }
        }
    }

    void draw_query_helper(node_id nid, bbox &query_bb) {
        if (query_bb.intersect(_qt.node_bbs[nid])) {

            if (empty != _qt.nodes[nid].nw) draw_query_helper(_qt.nodes[nid].nw, query_bb);
            if (empty != _qt.nodes[nid].ne) draw_query_helper(_qt.nodes[nid].ne, query_bb);
            if (empty != _qt.nodes[nid].sw) draw_query_helper(_qt.nodes[nid].sw, query_bb);
            if (empty != _qt.nodes[nid].se) draw_query_helper(_qt.nodes[nid].se, query_bb);

            _r.draw_rectangle(
                (vec2_t){_qt.node_bbs[nid].minx, _qt.node_bbs[nid].miny},
                (vec2_t){_qt.node_bbs[nid].maxx, _qt.node_bbs[nid].maxy},
                (rgba_t){255, 0, 0, 255});
        }
    }

    quadtree &_qt;
    Renderer &_r;
};

#endif