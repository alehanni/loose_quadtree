#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <cassert>

using node_id = std::uint32_t;
static constexpr node_id empty = node_id(-1);
static constexpr float inf = std::numeric_limits<float>::infinity();

struct point {
    float x, y;
};

struct bbox {
    float minx = inf;
    float miny = inf;
    float maxx = -inf;
    float maxy = -inf;

    void split_4(bbox &bb1, bbox &bb2, bbox &bb3, bbox &bb4) const {

        float midx = (minx + maxx) / 2.f;
        float midy = (miny + maxy) / 2.f;

        bb1.minx = minx;
        bb1.miny = miny;
        bb1.maxx = midx;
        bb1.maxy = midy;

        bb2.minx = midx;
        bb2.miny = miny;
        bb2.maxx = maxx;
        bb2.maxy = midy;

        bb3.minx = minx;
        bb3.miny = midy;
        bb3.maxx = midx;
        bb3.maxy = maxy;

        bb4.minx = midx;
        bb4.miny = midy;
        bb4.maxx = maxx;
        bb4.maxy = maxy;
    }

    bool intersect(bbox &other) {
        return (maxx > other.minx && minx < other.maxx
            && maxy > other.miny && miny < other.maxy);
    }

    bbox &operator |=(point rhs) {
        minx = std::min(minx, rhs.x);
        miny = std::min(miny, rhs.y);
        maxx = std::max(maxx, rhs.x);
        maxy = std::max(maxy, rhs.y);
        return *this;
    }

    friend bbox operator|(bbox lhs, point const& rhs) {
        lhs |= rhs;
        return lhs;
    }

    bbox &operator |=(bbox rhs) {
        minx = std::min(minx, rhs.minx);
        miny = std::min(miny, rhs.miny);
        maxx = std::max(maxx, rhs.maxx);
        maxy = std::max(maxy, rhs.maxy);
        return *this;
    }

    friend bbox operator|(bbox lhs, bbox const& rhs) {
        lhs |= rhs;
        return lhs;
    }
};

struct pointbox {
    point mid;
    bbox bb;
};

template<typename T>
using bbox_func = std::function<bbox(T&)>;

struct node {
    node_id nw = empty;
    node_id ne = empty;
    node_id sw = empty;
    node_id se = empty;
};

struct quadtree {
    bbox bb;
    node_id root;
    
    // per node data
    std::vector<node> nodes;
    std::vector<bbox> node_bbs;
    std::vector<std::uint32_t> node_points_begin;

    // per point data
    std::vector<pointbox> pointboxes;
    std::vector<std::uint32_t> indices;

    // region query
    void query(bbox query_bb, std::vector<std::uint32_t> &result);

protected:
    void query_helper(bbox query_bb, node_id nid, std::vector<std::uint32_t> &result);
};

void quadtree::query_helper(bbox query_bb, node_id nid, std::vector<std::uint32_t> &result) {
    if (query_bb.intersect(node_bbs[nid])) {

        bool is_not_leaf = false;
        if (empty != nodes[nid].nw && (is_not_leaf = true)) query_helper(query_bb, nodes[nid].nw, result);
        if (empty != nodes[nid].ne && (is_not_leaf = true)) query_helper(query_bb, nodes[nid].ne, result);
        if (empty != nodes[nid].sw && (is_not_leaf = true)) query_helper(query_bb, nodes[nid].sw, result);
        if (empty != nodes[nid].se && (is_not_leaf = true)) query_helper(query_bb, nodes[nid].se, result);

        if (!is_not_leaf) {
            auto i_begin = node_points_begin[nid];
            auto i_end = node_points_begin[nid + 1];

            // check intersections with pointboxes
            for (auto i = i_begin; i != i_end; i++) {
                if (query_bb.intersect(pointboxes[i].bb))
                    result.push_back(indices[i]);
            }

            //DrawRectangleLines( // ONLY USED FOR DEMO
            //node_bbs[nid].minx + 0.5f,
            //node_bbs[nid].miny + 0.5f,
            //node_bbs[nid].maxx - node_bbs[nid].minx + 0.5f,
            //node_bbs[nid].maxy - node_bbs[nid].miny + 0.5f,
            //BLUE
            //);
        } //else {
        //    DrawRectangleLines( // ONLY USED FOR DEMO
        //    node_bbs[nid].minx + 0.5f,
        //    node_bbs[nid].miny + 0.5f,
        //    node_bbs[nid].maxx - node_bbs[nid].minx + 0.5f,
        //    node_bbs[nid].maxy - node_bbs[nid].miny + 0.5f,
        //    RED
        //);
        //}
    }
}

void quadtree::query(bbox query_bb, std::vector<std::uint32_t> &result) {
    // check overlap with root node bb
    if (query_bb.intersect(node_bbs[root]))
        query_helper(query_bb, root, result);
}

node_id build_helper(quadtree &tree, bbox const &bb, pointbox *p_begin, pointbox *p_end, std::uint32_t depth_limit) {

    if (p_begin == p_end) return empty;

    node_id result = tree.nodes.size();
    tree.nodes.emplace_back();
    
    std::uint32_t idx = p_begin - &tree.pointboxes.front();
    assert(idx >= 0);
    assert(idx < tree.pointboxes.size());
    tree.node_points_begin.push_back(idx);

    // compute bounding box for node
    bbox node_bb = bbox();
    std::for_each(p_begin, p_end, [&node_bb](pointbox &pb) mutable {node_bb |= pb.bb;});
    tree.node_bbs.push_back(node_bb);

    if (p_begin + 1 == p_end || 0 == depth_limit) return result;

    bbox bb_nw, bb_ne, bb_sw, bb_se;
    bb.split_4(bb_nw, bb_ne, bb_sw, bb_se);

    point mid = {bb_nw.maxx, bb_nw.maxy};
    auto is_top = [mid](pointbox const &p){return p.mid.y < mid.y;};
    auto is_left = [mid](pointbox const &p){return p.mid.x < mid.x;};

    pointbox *split_y = std::partition(p_begin, p_end, is_top);
    pointbox *split_x_upper = std::partition(p_begin, split_y, is_left);
    pointbox *split_x_lower = std::partition(split_y, p_end, is_left);

    tree.nodes[result].nw = build_helper(tree, bb_nw, p_begin, split_x_upper, depth_limit - 1);
    tree.nodes[result].ne = build_helper(tree, bb_ne, split_x_upper, split_y, depth_limit - 1);
    tree.nodes[result].sw = build_helper(tree, bb_sw, split_y, split_x_lower, depth_limit - 1);
    tree.nodes[result].se = build_helper(tree, bb_se, split_x_lower, p_end, depth_limit - 1);

    return result;
}

static constexpr std::uint32_t MAX_QUADTREE_DEPTH = 4;

template<typename T>
quadtree build(std::vector<T> xs, std::vector<std::uint32_t> &indices, bbox_func<T> to_bbox) {

    assert(xs.size() > 0);

    auto to_pointbox = [to_bbox](T x){
        bbox bb = to_bbox(x);
        return (pointbox){
            {(bb.minx + bb.maxx) / 2.f, (bb.miny + bb.maxy) / 2.f},
            bb
        };
    };

    // convert type to bounding boxes and points
    std::vector<pointbox> pbs(xs.size());
    std::transform(&xs.front(), &xs.back(), &pbs.front(), to_pointbox);

    // compute bounding box of midpoints
    bbox big_box = bbox();
    std::for_each(&pbs.front(), &pbs.back(), [&big_box](pointbox &pb) mutable {big_box |= pb.mid;});

    quadtree qt;
    qt.pointboxes = std::move(pbs);
    qt.indices = std::move(indices);
    qt.root = build_helper(qt, big_box, &qt.pointboxes.front(), &qt.pointboxes.back(), MAX_QUADTREE_DEPTH);

    qt.node_points_begin.push_back(qt.pointboxes.size());

    return qt;
}

#endif