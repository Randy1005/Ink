#pragma once

#include <ink/ink.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace cpathgen_trace {

inline std::string trace_hash(
  const std::vector<size_t>& vertices,
  const std::vector<long long>& edges,
  const std::vector<long long>& weights) {
  uint64_t h = 1469598103934665603ull;
  for (auto v : vertices) {
    h ^= static_cast<uint64_t>(v);
    h *= 1099511628211ull;
  }
  for (auto e : edges) {
    h ^= static_cast<uint64_t>(e + 0x9e3779b97f4a7c15ull);
    h *= 1099511628211ull;
  }
  for (auto w : weights) {
    h ^= static_cast<uint64_t>(w + 0xbf58476d1ce4e5b9ull);
    h *= 1099511628211ull;
  }
  std::ostringstream os;
  os << std::hex << h;
  return os.str();
}

inline long long quantize_weight(float w) {
  return static_cast<long long>(std::llround(static_cast<double>(w) * 1000000000.0));
}

inline auto path_sort_key(const ink::Path& path) {
  std::vector<size_t> vertices;
  std::vector<long long> edges;
  std::vector<long long> weights;
  vertices.reserve(path.size());
  const ink::Point* prev = nullptr;
  for (const auto& point : path) {
    vertices.push_back(point.vert.id);
    if (point.incoming_edge_id >= 0 && prev != nullptr) {
      edges.push_back(point.incoming_edge_id);
      const auto step_weight = point.dist - prev->dist;
      weights.push_back(quantize_weight(step_weight));
    }
    prev = &point;
  }
  return std::tuple{path.weight, vertices, edges, weights, path.size()};
}

inline void sort_paths(std::vector<ink::Path>& paths) {
  std::sort(paths.begin(), paths.end(), [](const auto& a, const auto& b) {
    return path_sort_key(a) < path_sort_key(b);
  });
}

inline void write_jsonl(std::ostream& os, std::vector<ink::Path>& paths) {
  sort_paths(paths);
  os << std::setprecision(10);
  for (size_t rank = 0; rank < paths.size(); ++rank) {
    const auto& path = paths[rank];
    std::vector<size_t> vertices;
    std::vector<long long> edges;
    std::vector<long long> weights;
    std::vector<long long> weight_indices;
    vertices.reserve(path.size());
    edges.reserve(path.size() > 0 ? path.size() - 1 : 0);
    weights.reserve(path.size() > 0 ? path.size() - 1 : 0);
    weight_indices.reserve(path.size() > 0 ? path.size() - 1 : 0);
    const ink::Point* prev = nullptr;
    for (const auto& point : path) {
      vertices.push_back(point.vert.id);
      if (point.incoming_edge_id >= 0 && prev != nullptr) {
        edges.push_back(point.incoming_edge_id);
        const auto step_weight = point.dist - prev->dist;
        weights.push_back(quantize_weight(step_weight));
        weight_indices.push_back(point.incoming_weight_sel);
      }
      prev = &point;
    }
    os << "{";
    os << "\"rank\":" << rank << ",";
    os << "\"cost\":" << path.weight << ",";
    os << "\"slack\":" << path.weight << ",";
    os << "\"startpoint\":";
    if (path.empty()) os << "null"; else os << path.front().vert.id;
    os << ",\"endpoint\":";
    if (path.empty()) os << "null"; else os << path.back().vert.id;
    os << ",\"transition\":\"unknown\",";
    os << "\"vertex_trace\":[";
    for (size_t i = 0; i < vertices.size(); ++i) {
      if (i) os << ",";
      os << vertices[i];
    }
    os << "],\"edge_trace\":[";
    for (size_t i = 0; i < edges.size(); ++i) {
      if (i) os << ",";
      os << edges[i];
    }
    os << "],\"weight_trace\":[";
    for (size_t i = 0; i < weights.size(); ++i) {
      if (i) os << ",";
      os << std::fixed << std::setprecision(9) << (static_cast<double>(weights[i]) / 1000000000.0);
    }
    os << "],\"weight_index_trace\":[";
    for (size_t i = 0; i < weight_indices.size(); ++i) {
      if (i) os << ",";
      os << weight_indices[i];
    }
    os << "],";
    os << "\"trace_hash\":\"" << trace_hash(vertices, edges, weights) << "\"";
    os << "}\n";
  }
}

}  // namespace cpathgen_trace
