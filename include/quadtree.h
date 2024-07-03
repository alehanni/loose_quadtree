#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <algorithm>
#include <cstdint>
#include <cassert>

#include "utiltypes.h"

using node_id = std::uint32_t;
static constexpr node_id empty = node_id(-1);

box_t point_hull(auto *begin, auto *end) {
    box_t hull{inf, inf, -inf, -inf};
    for (auto it = begin; it != end; ++it) {
        auto p = *it;
        hull.min.x = std::min(hull.min.x, p.x);
        hull.min.y = std::min(hull.min.y, p.y);
        hull.max.x = std::max(hull.max.x, p.x);
        hull.max.y = std::max(hull.max.y, p.y);
    }
    return hull;
}

box_t box_hull(auto *begin, auto *end) {
    box_t hull{inf, inf, -inf, -inf};
    for (auto it = begin; it != end; ++it) {
        auto box = *it;
        hull.min.x = std::min(hull.min.x, box.min.x);
        hull.min.y = std::min(hull.min.y, box.min.y);
        hull.max.x = std::max(hull.max.x, box.max.x);
        hull.max.y = std::max(hull.max.y, box.max.y);
    }
    return hull;
}

void split_4(box_t const& bb, box_t &bb1, box_t &bb2, box_t &bb3, box_t &bb4) {

    vec2_t mid = (bb.min + bb.max) / 2.0;

    bb1.min.x = bb.min.x;
    bb1.min.y = bb.min.y;
    bb1.max.x = mid.x;
    bb1.max.y = mid.y;

    bb2.min.x = mid.x;
    bb2.min.y = bb.min.y;
    bb2.max.x = bb.max.x;
    bb2.max.y = mid.y;

    bb3.min.x = bb.min.x;
    bb3.min.y = mid.y;
    bb3.max.x = mid.x;
    bb3.max.y = bb.max.y;

    bb4.min.x = mid.x;
    bb4.min.y = mid.y;
    bb4.max.x = bb.max.x;
    bb4.max.y = bb.max.y;
}

struct node {
    node_id nw = empty;
    node_id ne = empty;
    node_id sw = empty;
    node_id se = empty;
};

struct point_box_t : public vec2_t, public box_t { };
static_assert( std::is_trivial_v<point_box_t> == true );

struct quadtree {
    box_t bb;
    node_id root;
    
    // per node data
    std::vector<node> nodes;
    std::vector<box_t> node_bbs;
    std::vector<std::uint32_t> node_points_begin;

    // per point data
    std::vector<point_box_t> pointboxes;
    std::vector<std::uint32_t> indices;

    // region query
    void query(box_t query_bb, std::vector<std::uint32_t> &result);

protected:
    void query_helper(box_t query_bb, node_id nid, std::vector<std::uint32_t> &result);
};

void quadtree::query_helper(box_t query_bb, node_id nid, std::vector<std::uint32_t> &result) {
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
                if (query_bb.intersect(pointboxes[i]))
                    result.push_back(indices[i]);
            }
        }
    }
}

void quadtree::query(box_t query_bb, std::vector<std::uint32_t> &result) {
    // check overlap with root node bb
    if (query_bb.intersect(node_bbs[root]))
        query_helper(query_bb, root, result);
}

node_id build_helper(quadtree &tree, box_t const &bb, point_box_t *p_begin, point_box_t *p_end, std::uint32_t depth_limit) {

    if (p_begin == p_end) return empty;

    node_id result = tree.nodes.size();
    tree.nodes.emplace_back();
    
    std::uint32_t idx = p_begin - &tree.pointboxes.front();
    assert(idx >= 0);
    assert(idx < tree.pointboxes.size());
    tree.node_points_begin.push_back(idx);

    // compute bounding box for node
    tree.node_bbs.push_back(box_hull(p_begin, p_end));

    if (p_begin + 1 == p_end || 0 == depth_limit) return result;

    box_t bb_nw, bb_ne, bb_sw, bb_se;
    split_4(bb, bb_nw, bb_ne, bb_sw, bb_se);

    vec2_t mid = {bb_nw.max.x, bb_nw.max.y};
    auto is_top = [mid](point_box_t const &pb){return pb.y < mid.y;};
    auto is_left = [mid](point_box_t const &pb){return pb.x < mid.x;};

    point_box_t *split_y = std::partition(p_begin, p_end, is_top);
    point_box_t *split_x_upper = std::partition(p_begin, split_y, is_left);
    point_box_t *split_x_lower = std::partition(split_y, p_end, is_left);

    tree.nodes[result].nw = build_helper(tree, bb_nw, p_begin, split_x_upper, depth_limit - 1);
    tree.nodes[result].ne = build_helper(tree, bb_ne, split_x_upper, split_y, depth_limit - 1);
    tree.nodes[result].sw = build_helper(tree, bb_sw, split_y, split_x_lower, depth_limit - 1);
    tree.nodes[result].se = build_helper(tree, bb_se, split_x_lower, p_end, depth_limit - 1);

    return result;
}

template<typename T>
using box_func = std::function<box_t(T&)>;

static constexpr std::uint32_t MAX_QUADTREE_DEPTH = 4;

template<typename T>
quadtree build(std::vector<T> xs, std::vector<std::uint32_t> &indices, box_func<T> to_box) {

    assert(xs.size() > 0);

    auto to_point_box = [to_box](T x){
        box_t bb = to_box(x);
        return (point_box_t){
            (bb.min.x + bb.max.x) / 2.0,
            (bb.min.y + bb.max.y) / 2.0,
            bb.min,
            bb.max
        };
    };
    
    // convert type to bounding boxes and points
    std::vector<point_box_t> pbs(xs.size());
    std::transform(xs.begin(), xs.end(), pbs.begin(), to_point_box);

    // compute bounding box of midpoints
    box_t big_box = point_hull(&pbs.front(), &pbs.back() + 1);

    quadtree qt;
    qt.pointboxes = std::move(pbs);
    qt.indices = std::move(indices);
    qt.root = build_helper(qt, big_box, &qt.pointboxes.front(), &qt.pointboxes.back() + 1, MAX_QUADTREE_DEPTH);

    qt.node_points_begin.push_back(qt.pointboxes.size());

    return qt;
}

#endif