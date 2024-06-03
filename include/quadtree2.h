#ifndef QUADTREE3_H
#define QUADTREE3_H

#include <vector>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <cassert>

using node_id = std::uint32_t;
static constexpr node_id null = node_id(-1);
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
using bbox_func = std::function<bbox&(T)>;

struct node {
    node_id nw = null;
    node_id ne = null;
    node_id sw = null;
    node_id se = null;
};

struct qtree {
    bbox bb;
    node_id root;
    
    // per node data
    std::vector<node> nodes;
    std::vector<bbox> node_bbs;
    std::vector<std::uint32_t> node_points_begin;

    // per point data
    std::vector<pointbox> pointboxes;

    void query(std::vector<std::uint32_t> indices);
};

node_id build_helper(qtree &tree, bbox const &bb, pointbox *p_begin, pointbox *p_end, std::uint32_t depth_limit) {

    if (p_begin == p_end) return null;

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
    auto top = [mid](pointbox const &p){return p.mid.y < mid.y;};
    auto left = [mid](pointbox const &p){return p.mid.x < mid.x;};

    pointbox *split_y = std::partition(p_begin, p_end, top);
    pointbox *split_x_upper = std::partition(p_begin, split_y, left);
    pointbox *split_x_lower = std::partition(split_y, p_end, left);

    tree.nodes[result].nw = build_helper(tree, bb_nw, p_begin, split_x_upper, depth_limit - 1);
    tree.nodes[result].ne = build_helper(tree, bb_ne, split_x_upper, split_y, depth_limit - 1);
    tree.nodes[result].sw = build_helper(tree, bb_sw, split_y, split_x_lower, depth_limit - 1);
    tree.nodes[result].se = build_helper(tree, bb_se, split_x_lower, p_end, depth_limit - 1);

    return result;
}

static constexpr std::uint32_t MAX_QUADTREE_DEPTH = 8;

template<typename T>
qtree build(std::vector<T> xs, bbox_func<T> to_bbox) {

    assert(xs.size() > 0);

    auto to_pointbox = [](T x){
        return (pointbox){
            to_bbox(x),
            {(bb.minx + bb.maxx) / 2.f, (bb.miny + bb.maxy) / 2.f}
        };
    };

    // convert type to bounding boxes and points
    std::vector<pointbox> pbs(xs.size());
    std::transform(xs.front(), xs.back(), pbs.front(), to_pointbox);

    // compute bounding box of midpoints
    bbox big_box = bbox();
    std::for_each(pbs.front(), pbs.back(), [&big_box](pointbox &pb) mutable {big_box |= pb.mid;});

    qtree qt;
    qt = build_helper(qt, big_box, qt.points.front(), qt.points.back(), MAX_QUADTREE_DEPTH);

    return qt;
}

#endif