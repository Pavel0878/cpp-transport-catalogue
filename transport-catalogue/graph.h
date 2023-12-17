#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

// диаграмма
namespace graph {

using VertexId = size_t;
using EdgeId = size_t; // Идентификатор края

// Край
template <typename Weight>
struct Edge {
    std::string name;
    size_t stop_count;
    VertexId from; // Идентификатор вершины
    VertexId to;   // Идентификатор вершины
    Weight weight; // Вес
};

// Ориентированный взвешенный график
template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>; // Список инцидентов
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>; // Диапазон падающих кромок

public:
    DirectedWeightedGraph() = default; // Ориентированный взвешенный график
    explicit DirectedWeightedGraph(size_t vertex_count); // Ориентированный взвешенный график
    EdgeId AddEdge(const Edge<Weight>& edge);  // Добавить ребро

    size_t GetVertexCount() const; // Получить количество вершин
    size_t GetEdgeCount() const;   // Получить количество ребер
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;  // Получите преимущество
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const; // Получить падающие ребра

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

//  Ориентированный взвешенный график
template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

//  Добавить ребро
template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

// Получить количество вершин
template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}
// Получить количество ребер
template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

//  Получите преимущество
template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

//  Получить падающие ребра
template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph