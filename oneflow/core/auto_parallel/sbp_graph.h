/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef SBP_GRAPH_H_
#define SBP_GRAPH_H_

#include "binary_set.h"
#include "oneflow/core/auto_parallel/sbp_node.h"
#include "sbp_edge.h"
#include <algorithm>
#include <unordered_map>
#include "algorithm_util.h"

namespace Algorithm {

template<class SbpSignature>
class SbpGraph {
 public:
  // Data Structure
  // All the nodes
  std::vector<SbpNode<SbpSignature> *> NodeList;

  // Over All Cost under current strategy
  double GraphCost = 0;
  // Limitation: Merged node should not have a number of Sbp Signature greater
  // than threshold.
  int32_t Threshold = 100;
  // The next id that we are going to use for nodes.
  int32_t NextId = 0;

  // functions
  SbpGraph();
  ~SbpGraph() {
    for (auto this_node : NodeList) { delete this_node; }
    NodeList.clear();
  }

  // Setup SbpSignature Candidates
  void AssembleSbpSignature(const std::function<int32_t()> &CalcOrderValue4SbpSig,
                            std::vector<SbpSignature *> GlobalSbpSignatureList);

  // Use our algorithm to decide SbpSignature for each op-node
  void DecideSbpSignature();

  // Compute Cost before elimination
  void ComputeInitialCost(
      const std::function<double(SbpNode<SbpSignature> *, SbpSignature *, SbpNode<SbpSignature> *,
                                 SbpSignature *)> &SbpInferHint4Ibn,
      const std::function<double(SbpNode<SbpSignature> *, SbpSignature *)> &SbpComputationCost);

  // Randomly assign a SbpSignature strategy
  void RandomSbpSignature();

  // Compute Cost for current strategy
  double ComputeCost();

  // Generate a node
  SbpNode<SbpSignature> *GenerateNode();

  // Remove a node from nodelist
  void RemoveFromNodeList(SbpNode<SbpSignature> *this_node);

  // Check and eliminate one node with only one degree-in and one degree-out
  int32_t NodeElimination(SbpNode<SbpSignature> *this_node);
  // Merge all parallel edges with given StartNode and EndNode
  int32_t EdgeElimination(SbpNode<SbpSignature> *this_node);
  // Ckeck and eliminate one child node
  int32_t ChildElimination(SbpNode<SbpSignature> *this_node);

  // Merge all parallel edges & Check and eliminate all nodes with only one
  // degree-in and one degree-out
  int32_t NodeAndEdgeEliminations();

  // Merge two nodes
  int32_t NodeMerging(SbpNode<SbpSignature> *first, SbpNode<SbpSignature> *second);
  // Select two nodes and merge them
  int32_t PickAndMerge();

  // Finalize Sbp Cost for the whole graph
  void FinalizeSbp();

  // Use Greedy Strategy to decide Sbp for Nodes in NodeList. Should be used
  // after we have a initial strategy.
  // Set ForceNode to be true will only use GreedyStrategy on Nodes.
  double GreedyStrategy(bool ForceNode);
  // Use greedy strategy on the one ring neighborhood with the maximum number of points nbh_num.
  double GreedyStrategy(int32_t nbh_num = 4);

  // Use brute force to search for a strategy with minimum cost for a neighborhood
  double NbhGreedyStrategy(std::vector<int32_t> &nbh_id2NodeListId);

  // Set Threshold for SbpNode Merging
  void SetThreshold(int32_t thrhld) { Threshold = thrhld; }

  // Clip an edge, remove it from graph
  // Clipping an edge will also delete the nodes and edges contained in this edge. Though not
  // sufferring from any compiling and runtime bugs, clipping an edge on a shrunk graph is not
  // recommanded. We should carefully think about it before any clipping.
  void ClipEdge(SbpEdge<SbpSignature> *this_edge);

  // Detect all the overlaps and then adjust copy cost correspondingly.
  void DetectAdjustOverlap(double CostRatio);

  // Compute the minimum and maximum layer of each node in the graph
  void ComputeLayer(oneflow::HashMap<std::string, SbpNode<SbpSignature> *> &op_name2sbp_node);

 private:
  void DFS_AddNbhCost(std::vector<int32_t> &nbh_id2NodeListId,
                      std::unordered_map<int32_t, int32_t> &NodeListId2nbh_id,
                      std::vector<int32_t> &order2nbh_id, std::vector<int32_t> &nbh_id2order,
                      std::vector<double> &order2AccMinInNbhCost,
                      std::vector<std::vector<double>> &OutNbhCosts,
                      std::vector<std::vector<int32_t>> &nbh_id2order2sbp_id,
                      std::vector<int32_t> &MinSbpSignatureId, double &MinCost, int32_t order,
                      double CurrCost);

#ifdef PRINT_GRAPH_
  void PrintGraph();
  void PrintSbpSigs();
#endif  // PRINT_GRAPH_

#ifdef DEBUG_ALGORITHM_

  // Compute Cost for current startegy with original graph
  double ComputeOriginCost();

  // Original NodeList
  std::vector<SbpNode<SbpSignature> *> OriginalNodeList;

  // get ready for Topological sorting
  void InitTopologicalSort() {
    for (const auto &this_node : NodeList) { this_node->CurrDeg = this_node->EdgesIn.size(); }
  }

#endif  // DEBUG_ALGORITHM_

};  // class SbpGraph

}  // namespace Algorithm

// function in cpp. Should be put in one file due to use of template
// Otherwise we will need to declare specific template at the end of cpp file.
namespace Algorithm {

// Generate a node
template<class SbpSignature>
SbpNode<SbpSignature> *SbpGraph<SbpSignature>::GenerateNode() {
  SbpNode<SbpSignature> *this_node = new SbpNode<SbpSignature>();
  NodeList.emplace_back(this_node);
  this_node->id = NextId++;
  this_node->NodeListId = NodeList.size() - 1;
  return this_node;
}

template<class SbpSignature>
void SbpGraph<SbpSignature>::RemoveFromNodeList(SbpNode<SbpSignature> *this_node) {
  if (this_node->NodeListId < 0) return;
  NodeList.back()->NodeListId = this_node->NodeListId;
  RemoveFrom<SbpNode<SbpSignature> *>(NodeList, this_node->NodeListId);
  this_node->NodeListId = -1;
}

#ifndef RANDOM_GENERATOR_
template<class SbpSignature>
SbpGraph<SbpSignature>::SbpGraph() {}
#endif  // RANDOM_GENERATOR_

template<class SbpSignature>
void SbpGraph<SbpSignature>::ComputeInitialCost(
    const std::function<double(SbpNode<SbpSignature> *, SbpSignature *, SbpNode<SbpSignature> *,
                               SbpSignature *)> &SbpInferHint4Ibn,
    const std::function<double(SbpNode<SbpSignature> *, SbpSignature *)> &SbpComputationCost) {
  for (const auto &this_node : NodeList) {
    this_node->ComputeCost(SbpComputationCost);
    this_node->OriginCost = this_node->Cost;
    for (const auto &edge_out : this_node->EdgesOut) {
      edge_out->ComputeCost(SbpInferHint4Ibn);
      this_node->OriginCostOut.emplace_back(edge_out->Cost);
    }
  }
};

template<class SbpSignature>
void SbpGraph<SbpSignature>::AssembleSbpSignature(
    const std::function<int32_t()> &CalcOrderValue4SbpSig,
    std::vector<SbpSignature *> GlobalSbpSignatureList) {
  for (const auto &this_node : NodeList) {
    this_node->InitializeSbp(CalcOrderValue4SbpSig, GlobalSbpSignatureList);
  }
};

template<class SbpSignature>
void SbpGraph<SbpSignature>::RandomSbpSignature() {
  for (const auto &this_node : NodeList) {
#ifdef USE_SBP_COLLECTOR_
    if (this_node->SbpSignatureList.size() > 0)
      this_node->FinalSbpSignatureId = rand() % this_node->SbpSignatureList.size();
    else
      this_node->FinalSbpSignatureId = rand() % this_node->ParallelCandidates.size();
#else   // USE_SBP_COLLECTOR_
    this_node->FinalSbpSignatureId = rand() % this_node->SbpSignatureList.size();
#endif  // USE_SBP_COLLECTOR_
  }
};

template<class SbpSignature>
double SbpGraph<SbpSignature>::ComputeCost() {
  GraphCost = 0;
  for (const auto &this_node : NodeList) {
    int32_t this_id = this_node->FinalSbpSignatureId;

    GraphCost += this_node->Cost[this_id];
    for (const auto &edge_out : this_node->EdgesOut) {
      GraphCost += edge_out->Cost[this_id][edge_out->EndNode->FinalSbpSignatureId];
    }
  }
  return GraphCost;
}

template<class SbpSignature>
int32_t SbpGraph<SbpSignature>::NodeElimination(SbpNode<SbpSignature> *this_node) {
  if (this_node->EdgesIn.size() + this_node->EdgesOut.size() == 2) {
    std::vector<SbpNode<SbpSignature> *> TwoNode;
    for (const auto &one_edge : this_node->EdgesIn) TwoNode.emplace_back(one_edge->StartNode);
    for (const auto &one_edge : this_node->EdgesOut) TwoNode.emplace_back(one_edge->EndNode);

    // If a node is pointing to itself, could happen when shrink from a circle
    if (TwoNode[0] == TwoNode[1]) {
      int32_t EliminationNumber = 0;
      if (this_node->EdgesOut.empty())
        EliminationNumber += EdgeElimination(TwoNode[0]);
      else
        EliminationNumber += EdgeElimination(this_node);

      EliminationNumber += ChildElimination(this_node);
      return EliminationNumber;
    }

    std::vector<SbpEdge<SbpSignature> *> TwoEdge(this_node->EdgesIn);
    TwoEdge.insert(TwoEdge.end(), this_node->EdgesOut.begin(), this_node->EdgesOut.end());

    int32_t EdgesInSize = this_node->EdgesIn.size();

    SbpEdge<SbpSignature> *e =
        new SbpEdge<SbpSignature>(TwoNode[0], this_node, TwoNode[1], TwoEdge[0], TwoEdge[1]);
    e->SummerizeCost();
    // check and remove the edge_in with new edge in graph
    for (int32_t i = 0; i < EdgesInSize; i++) {
      CheckAndRemoveFrom<SbpEdge<SbpSignature> *>(TwoNode[i]->EdgesOut, TwoEdge[i]);
    }
    // check and remove the edge_out with new edge in graph
    for (int32_t i = EdgesInSize; i < 2; i++) {
      CheckAndRemoveFrom<SbpEdge<SbpSignature> *>(TwoNode[i]->EdgesIn, TwoEdge[i]);
    }
    // Let e take control of EdgeList completely by disconnecting MidNode
    e->MidNode->EdgesOut.clear();
    e->MidNode->EdgesIn.clear();

    // Insert new compound edge into graph
    TwoNode[0]->EdgesOut.emplace_back(e);
    TwoNode[1]->EdgesIn.emplace_back(e);

    // eliminate the node from graph by swaping with the last element and
    // popping
    RemoveFromNodeList(this_node);

    // successfully eliminate this node
    return 1;
  }
  // can not eliminate this node
  return 0;
}

template<class SbpSignature>
int32_t SbpGraph<SbpSignature>::NodeAndEdgeEliminations() {
  // Total elimination number
  int32_t TtlElmNum = 0;
  int32_t EliminationsNumber = 1;
  // repeat these kinds of elimination until stuck
  while (EliminationsNumber > 0) {
    EliminationsNumber = 0;
    for (int32_t i = NodeList.size() - 1; i >= 0; i--) {
      EliminationsNumber += NodeElimination(NodeList[i]);
    }

    for (int32_t i = NodeList.size() - 1; i >= 0; i--) {
      EliminationsNumber += EdgeElimination(NodeList[i]);
    }

    for (int32_t i = NodeList.size() - 1; i >= 0; i--) {
      EliminationsNumber += ChildElimination(NodeList[i]);
    }

    if (EliminationsNumber == 0 && NodeList.size() > 2) {
      EliminationsNumber += PickAndMerge();
      for (int32_t i = NodeList.size() - 1; i >= 0; i--) {
        EliminationsNumber += EdgeElimination(NodeList[i]);
      }
    }

    TtlElmNum += EliminationsNumber;
  }

  return TtlElmNum;
}

template<class SbpSignature>
int32_t LookForParallelEdge(SbpEdge<SbpSignature> *&e, SbpNode<SbpSignature> *start_node,
                            SbpNode<SbpSignature> *end_node, bool ifReverse, int32_t stopsign) {
  // elimination edges with specific start node and end node in
  // start_node->EdgesOut from index stopsign to the end.
  // start_node->EdgesOut[Stopsign] not included and need special treatment
  // after this process.
  int32_t EliminationsNumber = 0;
  for (int32_t j = start_node->EdgesOut.size() - 1; j > stopsign; j--) {
    if (end_node == start_node->EdgesOut[j]->EndNode) {
      if (!e) {
        if (ifReverse)
          e = new SbpEdge<SbpSignature>(end_node, start_node);
        else
          e = new SbpEdge<SbpSignature>(start_node, end_node);
      }
      // edge elimination
      e->EdgeList.emplace_back(start_node->EdgesOut[j]);
      EliminationsNumber++;
      RemoveFrom<SbpEdge<SbpSignature> *>(start_node->EdgesOut, j);
    }
  }
  return EliminationsNumber;
}

// Remove all edges with (start_node -> end_node) from EdgesIn of end_node
template<class SbpSignature>
void RemoveFromEdgesIn(SbpNode<SbpSignature> *start_node, SbpNode<SbpSignature> *end_node) {
  for (int32_t i = end_node->EdgesIn.size() - 1; i >= 0; i--) {
    if (start_node == end_node->EdgesIn[i]->StartNode) {
      RemoveFrom<SbpEdge<SbpSignature> *>(end_node->EdgesIn, i);
    }
  }
}

template<class SbpSignature>
int32_t SbpGraph<SbpSignature>::EdgeElimination(SbpNode<SbpSignature> *this_node) {
  int32_t EliminationsNumber = 0;

  for (int32_t i = 0; i < this_node->EdgesOut.size(); i++) {
    SbpEdge<SbpSignature> *e = NULL;
    // Find and delete Parallel Edges from EdgesOut
    EliminationsNumber +=
        LookForParallelEdge<SbpSignature>(e, this_node, this_node->EdgesOut[i]->EndNode, false, i);
    EliminationsNumber +=
        LookForParallelEdge<SbpSignature>(e, this_node->EdgesOut[i]->EndNode, this_node, true, -1);
    if (e) {
      // Delete Parallel Edges from EdgesIn
      RemoveFromEdgesIn<SbpSignature>(this_node, e->EndNode);
      RemoveFromEdgesIn<SbpSignature>(e->EndNode, this_node);
      // Add the compound edge
      e->EdgeList.emplace_back(this_node->EdgesOut[i]);
      this_node->EdgesOut[i] = e;
      e->SummerizeCost();
      e->EndNode->EdgesIn.emplace_back(e);
    }
  }
  return EliminationsNumber;
}

template<class SbpSignature>
int32_t SbpGraph<SbpSignature>::ChildElimination(SbpNode<SbpSignature> *this_node) {
  if (this_node->EdgesIn.size() + this_node->EdgesOut.size() == 1) {
    if (this_node->EdgesIn.size()) {
      // edge in graph: father -> this_node
      SbpNode<SbpSignature> *father = this_node->EdgesIn[0]->StartNode;
      father->Children.emplace_back(this_node);
      CheckAndRemoveFrom<SbpEdge<SbpSignature> *>(father->EdgesOut, this_node->EdgesIn[0]);
      father->SummerizeCost();
    } else {
      // edge in graph: this_node -> father
      SbpNode<SbpSignature> *father = this_node->EdgesOut[0]->EndNode;
      father->Children.emplace_back(this_node);
      CheckAndRemoveFrom<SbpEdge<SbpSignature> *>(father->EdgesIn, this_node->EdgesOut[0]);
      father->SummerizeCost();
    }

    // eliminate this node from global node list
    RemoveFromNodeList(this_node);

    // successfully eliminate this node
    return 1;
  }
  // can not eliminate this node
  return 0;
}

// Merge two nodes
template<class SbpSignature>
int32_t SbpGraph<SbpSignature>::NodeMerging(SbpNode<SbpSignature> *first,
                                            SbpNode<SbpSignature> *second) {
  SbpNode<SbpSignature> *new_node = new SbpNode<SbpSignature>(first, second);

  // Adjust NodeList
  RemoveFromNodeList(first);
  RemoveFromNodeList(second);

  new_node->NodeListId = NodeList.size();
  NodeList.emplace_back(new_node);

  new_node->id = NextId++;

  return 1;
}

template<class SbpSignature>
void SbpGraph<SbpSignature>::FinalizeSbp() {
  for (const auto &this_node : NodeList) { this_node->FinalizeSbp(); }
}

template<class SbpSignature>
double SbpGraph<SbpSignature>::GreedyStrategy(bool ForceNode) {
  // Overall, this function should be replaced by GreedyStrategy(nbh_num);
  // Total Cost Reduce & Cost Reduce for one loop
  double TtlCostRdc = 0, CostRdc;
  for (int32_t step = NodeList.size(); step >= 0; step--) {
    CostRdc = 0;
    for (SbpNode<SbpSignature> *this_node : NodeList) {
      // Use GreedyStrategy on Nodes if there is one node left for this
      // connected component. Otherwise, Use GreedyStrategy on Edges.
      if (ForceNode || this_node->EdgesIn.size() + this_node->EdgesOut.size() == 0) {
        CostRdc += this_node->GreedyStrategy();
      } else {
        // GreedyStrategy on Edges.
        for (SbpEdge<SbpSignature> *this_edge : this_node->EdgesOut) {
          double second_rdc = this_edge->GreedyStrategy();
          CostRdc += second_rdc;
        }
      }
    }
    if (CostRdc == 0) break;
    TtlCostRdc += CostRdc;
  }
  return TtlCostRdc;
}

template<class SbpSignature>
double SbpGraph<SbpSignature>::GreedyStrategy(int32_t nbh_num) {
  // nbh_num is the maximum number of neighborhood to adjust sbp strategy in each step
  // Total Cost Reduce & Cost Reduce for one loop
  double TtlCostRdc = 0, CostRdc;
  // A global buffer to store part of the one ring neighborhood.
  std::vector<int32_t> nbh_id2NodeListId;
  if (nbh_num > 1)
    nbh_id2NodeListId.resize(nbh_num);
  else
    nbh_id2NodeListId.resize(1);

  for (int32_t step = NodeList.size(); step >= 0; step--) {
    CostRdc = 0;
    for (SbpNode<SbpSignature> *this_node : NodeList) {
      if (nbh_num <= 1) {
        // Greedy strategy on nodes
        nbh_id2NodeListId[0] = this_node->NodeListId;
        CostRdc += NbhGreedyStrategy(nbh_id2NodeListId);
      } else {
        // Use GreedyStrategy on the one ring neighborhood of this node.
        std::vector<int32_t> nbh_1ring;
        this_node->OneRingNeighborhood(nbh_1ring);
        if (nbh_1ring.size() <= nbh_num) {
          CostRdc += NbhGreedyStrategy(nbh_1ring);
        } else {
          // Use GreedyStrategy on part of the one ring neighborhood.
          // Loop through the neighborhood. Each loop should contain the centroid.

          // Initialize part of the one ring neighborhood
          int32_t nbh_1ring_id = nbh_1ring.size() - nbh_num;
          for (int32_t nbh_id = 1; nbh_id < nbh_num; ++nbh_id) {
            nbh_id2NodeListId[nbh_id] = nbh_1ring[++nbh_1ring_id];
          }
          // loop through the one ring neighborhood
          int32_t nbh_id = 0;
          for (nbh_1ring_id = 0; nbh_1ring_id < nbh_1ring.size(); ++nbh_1ring_id) {
            nbh_id2NodeListId[nbh_id] = nbh_1ring[nbh_1ring_id];
            CostRdc += NbhGreedyStrategy(nbh_id2NodeListId);
            // nbh_id for the next step
            if (++nbh_id >= nbh_num) nbh_id = 1;
          }
        }
      }
    }
    if (CostRdc == 0) break;
    TtlCostRdc += CostRdc;
  }
  return TtlCostRdc;
}

template<class SbpSignature>
void SbpGraph<SbpSignature>::DFS_AddNbhCost(std::vector<int32_t> &nbh_id2NodeListId,
                                            std::unordered_map<int32_t, int32_t> &NodeListId2nbh_id,
                                            std::vector<int32_t> &order2nbh_id,
                                            std::vector<int32_t> &nbh_id2order,
                                            std::vector<double> &order2AccMinInNbhCost,
                                            std::vector<std::vector<double>> &OutNbhCosts,
                                            std::vector<std::vector<int32_t>> &nbh_id2order2sbp_id,
                                            std::vector<int32_t> &MinSbpSignatureId,
                                            double &MinCost, int32_t order, double CurrCost) {
  // We have finished visiting the neighborhood
  if (order >= nbh_id2NodeListId.size()) {
    if (CurrCost < MinCost) {
      MinCost = CurrCost;
      for (int32_t nbh_id = 0; nbh_id < nbh_id2NodeListId.size(); nbh_id++) {
        MinSbpSignatureId[nbh_id] = NodeList[nbh_id2NodeListId[nbh_id]]->FinalSbpSignatureId;
      }
    }
    return;
  }
  // Pruning, remove all those branch with large cost
  if (CurrCost + order2AccMinInNbhCost[order] >= MinCost) return;
  // DFS in the next order
  int32_t nbh_id = order2nbh_id[order];
  Algorithm::SbpNode<SbpSignature> *sbp_node = NodeList[nbh_id2NodeListId[nbh_id]];
  for (int32_t sbp_id : nbh_id2order2sbp_id[nbh_id]) {
    sbp_node->FinalSbpSignatureId = sbp_id;
    DFS_AddNbhCost(nbh_id2NodeListId, NodeListId2nbh_id, order2nbh_id, nbh_id2order,
                   order2AccMinInNbhCost, OutNbhCosts, nbh_id2order2sbp_id, MinSbpSignatureId,
                   MinCost, order + 1,
                   CurrCost + OutNbhCosts[nbh_id][sbp_id]
                       + sbp_node->EvalInNbhCost(NodeListId2nbh_id, nbh_id2order));
  }
}

// Use brute force to search for a strategy with minimum cost for a neighborhood
template<class SbpSignature>
double SbpGraph<SbpSignature>::NbhGreedyStrategy(std::vector<int32_t> &nbh_id2NodeListId) {
  // number of nodes in the neighborhood
  int32_t num_nbh = nbh_id2NodeListId.size();
  // mapping from the NodeListId to the id in the nbh_id2NodeListId
  std::unordered_map<int32_t, int32_t> NodeListId2nbh_id;
  InverseFunction<int32_t>(nbh_id2NodeListId, NodeListId2nbh_id);
  // a sbp signature id set minimizing the overall cost, store the original one as default
  std::vector<int32_t> MinSbpSignatureId(num_nbh);
  for (int32_t nbh_id = 0; nbh_id < num_nbh; nbh_id++) {
    MinSbpSignatureId[nbh_id] = NodeList[nbh_id2NodeListId[nbh_id]]->FinalSbpSignatureId;
  }

  // pre-compute and store the cost between neighborhood and outside nodes under different sbp for
  // each node within the neighborhood
  std::vector<std::vector<double>> OutNbhCosts(num_nbh);
  for (int32_t nbh_id = 0; nbh_id < num_nbh; nbh_id++) {
    Algorithm::SbpNode<SbpSignature> *sbp_node = NodeList[nbh_id2NodeListId[nbh_id]];
    OutNbhCosts[nbh_id].resize(sbp_node->Cost.size());
    for (int32_t sbp_id = sbp_node->Cost.size() - 1; sbp_id >= 0; sbp_id--) {
      sbp_node->FinalSbpSignatureId = sbp_id;
      OutNbhCosts[nbh_id][sbp_id] = sbp_node->EvalOutNbhCost(NodeListId2nbh_id);
    }
  }
  // pre-compute and store the order of the OutNbhCosts
  std::vector<std::vector<int32_t>> nbh_id2order2sbp_id(num_nbh);
  for (int32_t nbh_id = 0; nbh_id < num_nbh; nbh_id++) {
    DecideOrder(OutNbhCosts[nbh_id], nbh_id2order2sbp_id[nbh_id],
                [](double a, double b) { return a < b; });
  }

  // Decide the order to go through the neighborhood.
  // Should visit those nodes with a larger difference in the out cost first.
  std::vector<double> OutNbhCostDiff(num_nbh);
  for (int32_t nbh_id = 0; nbh_id < num_nbh; nbh_id++) {
    OutNbhCostDiff[nbh_id] =
        *std::max_element(OutNbhCosts[nbh_id].begin(), OutNbhCosts[nbh_id].end())
        - *std::min_element(OutNbhCosts[nbh_id].begin(), OutNbhCosts[nbh_id].end());
  }
  std::vector<int32_t> order2nbh_id;
  DecideOrder(OutNbhCostDiff, order2nbh_id, [](double a, double b) { return a > b; });
  // Find the inverse map of order
  std::vector<int32_t> nbh_id2order;
  InverseOrder(order2nbh_id, nbh_id2order);

  // Current Cost, Minimum Cost, Cost with original sbp
  double MinCost, OrgCost = 0;
  // Recover original sbp
  for (int32_t nbh_id = 0; nbh_id < num_nbh; nbh_id++) {
    NodeList[nbh_id2NodeListId[nbh_id]]->FinalSbpSignatureId = MinSbpSignatureId[nbh_id];
  }
  // Compute cost with original sbp
  for (int32_t nbh_id = 0; nbh_id < num_nbh; nbh_id++) {
    Algorithm::SbpNode<SbpSignature> *sbp_node = NodeList[nbh_id2NodeListId[nbh_id]];
    OrgCost += OutNbhCosts[nbh_id][MinSbpSignatureId[nbh_id]];
    OrgCost += sbp_node->EvalInNbhCost(NodeListId2nbh_id, nbh_id2order);
  }
  MinCost = OrgCost;
  // Accumulate minimum cost form the current node to the end of the neighborhood node list.
  // The accumulated cost include the current node.
  std::vector<double> order2AccMinInNbhCost(num_nbh);
  order2AccMinInNbhCost[num_nbh - 1] = *std::min_element(
      OutNbhCosts[order2nbh_id[num_nbh - 1]].begin(), OutNbhCosts[order2nbh_id[num_nbh - 1]].end());
  for (int32_t order = num_nbh - 2; order >= 0; order--) {
    int32_t nbh_id = order2nbh_id[order];
    order2AccMinInNbhCost[order] =
        order2AccMinInNbhCost[order + 1]
        + *std::min_element(OutNbhCosts[nbh_id].begin(), OutNbhCosts[nbh_id].end())
        + NodeList[nbh_id2NodeListId[nbh_id]]->EvalMinInNbhCost(NodeListId2nbh_id, nbh_id2order);
  }
  // Use brute force (DFS) to adjust for the best strategy in the neighborhood.
  DFS_AddNbhCost(nbh_id2NodeListId, NodeListId2nbh_id, order2nbh_id, nbh_id2order,
                 order2AccMinInNbhCost, OutNbhCosts, nbh_id2order2sbp_id, MinSbpSignatureId,
                 MinCost, 0, 0);
  // Use the sbp strategy with minimum cost
  for (int32_t nbh_id = 0; nbh_id < num_nbh; nbh_id++) {
    NodeList[nbh_id2NodeListId[nbh_id]]->FinalSbpSignatureId = MinSbpSignatureId[nbh_id];
  }

  return MinCost - OrgCost;
}

// Select and Merge two nodes
template<class SbpSignature>
int32_t SbpGraph<SbpSignature>::PickAndMerge() {
  if (NodeList.size() < 4) return 0;
  std::vector<BinarySet> NodeBinarySets(NodeList.size());
  for (int32_t i = 0; i < NodeList.size(); i++) {
    // Transfer edge to binary set
    NodeBinarySets[i].Initialize(NodeList.size());
    NodeBinarySets[i].AddEntry(i);
    for (const SbpEdge<SbpSignature> *edge_in : NodeList[i]->EdgesIn) {
      NodeBinarySets[i].AddEntry(edge_in->StartNode->NodeListId);
    }
    for (const SbpEdge<SbpSignature> *edge_out : NodeList[i]->EdgesOut) {
      NodeBinarySets[i].AddEntry(edge_out->StartNode->NodeListId);
    }
  }
  // Find two nodes with largest common subset
  // buffer of binary set
  BinarySet BuffBnrSet(NodeList.size());
  // Number of common edges
  int32_t MaxCommEdgeNum = 0, CurrCommEdgeNum;
  int32_t MinNodePair[2];
  // Number of Sbp Signature in merged node
  int32_t MinSbpNum = 0, CurrSbpNum;
  for (int32_t i = 0; i < NodeList.size(); i++) {
    for (int32_t j = i + 1; j < NodeList.size(); j++) {
      CurrSbpNum = NodeList[i]->Cost.size() * NodeList[j]->Cost.size();
      if (CurrSbpNum <= Threshold) {
        NodeBinarySets[i].IntersectionTo(NodeBinarySets[j], BuffBnrSet);
        CurrCommEdgeNum = BuffBnrSet.Total();
        if (CurrCommEdgeNum > MaxCommEdgeNum
            || (CurrCommEdgeNum == MaxCommEdgeNum && CurrSbpNum < MinSbpNum)) {
          MinNodePair[0] = i;
          MinNodePair[1] = j;
          MaxCommEdgeNum = CurrCommEdgeNum;
          MinSbpNum = CurrSbpNum;
        }
      }
    }
  }
  if (MaxCommEdgeNum > 0)
    return NodeMerging(NodeList[MinNodePair[0]], NodeList[MinNodePair[1]]);
  else
    return 0;
}

// Clip an edge, remove it from graph
template<class SbpSignature>
void SbpGraph<SbpSignature>::ClipEdge(SbpEdge<SbpSignature> *this_edge) {
  CheckAndRemoveFrom<SbpEdge<SbpSignature> *>(this_edge->EndNode->EdgesIn, this_edge);
  CheckAndRemoveFrom<SbpEdge<SbpSignature> *>(this_edge->StartNode->EdgesOut, this_edge);
  delete this_edge;
}

// Detect all the overlaps and then adjust copy cost correspondingly.
template<class SbpSignature>
void SbpGraph<SbpSignature>::DetectAdjustOverlap(double CostRatio) {
  // Find the maximum layer number in the graph
  int32_t max_layer_num = -1;
  for (const auto &this_node : NodeList) {
    if (this_node->MinLayer > max_layer_num) max_layer_num = this_node->MinLayer;
  }
  max_layer_num++;
  // Prestore the first and second maximum computation cost for each layer
  // In a layer, each operator will provide the mininum element in the array Cost.
  // max_1_comp_cost[i] >= max_2_comp_cost[i] >= the rest computation cost on the i-th layer
  std::vector<double> max_1_comp_cost(max_layer_num, -1.0);
  std::vector<double> max_2_comp_cost(max_layer_num, -1.0);
  // Prestore the id of the op with the maximum computation cost in the i-th layer
  std::vector<int32_t> max_1_id(max_layer_num);

  for (const auto &this_node : NodeList) {
    int32_t lay_num = this_node->MinLayer;
    if (lay_num < 0) continue;
    double comp_cost = this_node->GetMinCost();
    if (comp_cost > max_2_comp_cost[lay_num]) {
      if (comp_cost > max_1_comp_cost[lay_num]) {
        max_2_comp_cost[lay_num] = max_1_comp_cost[lay_num];
        max_1_comp_cost[lay_num] = comp_cost;
        max_1_id[lay_num] = this_node->id;
      } else {
        max_2_comp_cost[lay_num] = comp_cost;
      }
    }
  }

  // Detect all the overlaps
  for (const auto &this_node : NodeList) {
    int32_t lay_num = this_node->MinLayer;
    // Skip proxy nodes and single node in one layer
    // if (lay_num < 0 || max_2_comp_cost[lay_num] < 0.0) continue;
    // // Detect overlap. We do not spread it since we only adjust outcoming edges.
    // double min_ratio = std::min(CostRatio, 0.5);
    if (lay_num == 174 || lay_num == 179 || lay_num == 192)
      this_node->DetectSpreadOverlap(1e10, 1e10, 1, 0.001);
    // this_node->DetectSpreadOverlap(max_1_comp_cost[lay_num], max_2_comp_cost[lay_num],
    //                                max_1_id[lay_num], min_ratio);
  }
  // adjust copy cost correspondingly.
  for (const auto &this_node : NodeList) {
    for (const auto &this_edge : this_node->EdgesIn) { this_edge->AdjustOverlapCost(); }
  }
}

// Compute the minimum and maximum layer of each node in the graph
template<class SbpSignature>
void SbpGraph<SbpSignature>::ComputeLayer(
    oneflow::HashMap<std::string, SbpNode<SbpSignature> *> &op_name2sbp_node) {
  for (SbpNode<SbpSignature> *this_node : NodeList) { this_node->GetMinLayer(op_name2sbp_node); }
  for (SbpNode<SbpSignature> *this_node : NodeList) { this_node->SpreadMaxLayer(op_name2sbp_node); }
  for (SbpNode<SbpSignature> *this_node : NodeList) { this_node->LiftMaxLayer(); }
}

#ifdef RANDOM_GENERATOR_

template<class SbpSignature>
SbpGraph<SbpSignature>::SbpGraph() {
  // It's a random generator

  // generate node candidates
  // std::vector<std::vector<std::shared_ptr<SbpNode<SbpSignature>>>> AllNodes;
  std::vector<std::vector<SbpNode<SbpSignature> *>> AllNodes;
  AllNodes.resize(15 + rand() % 9);
  AllNodes[0].resize(6 + rand() % 12, NULL);

  for (int32_t i = 1; i < AllNodes.size(); i++) { AllNodes[i].resize(9 + rand() % 15, NULL); }

  // set data size
  int32_t DataSize = rand() % 30000 + 30000;

  // initialize starting nodes
  for (int32_t j = 0; j < AllNodes[0].size(); j++) {
    // std::shared_ptr<SbpNode<SbpSignature>> n_ =
    // std::make_shared<SbpNode<SbpSignature>>(DataSize, rand() % 1000 + 40, 0);
    AllNodes[0][j] = new SbpNode<SbpSignature>(DataSize, rand() % 1000 + 40, 0);
  }

  // connect them as a DAG
  int depth = AllNodes.size();
  for (int32_t i = 0; i < AllNodes.size(); i++) {
    for (int32_t j = 0; j < AllNodes[i].size(); j++) {
      if (AllNodes[i][j]) {
        // generate number of out-edge n
        int32_t p = rand() % 100;
        int32_t n;
        if (p > 50)
          n = 1;
        else if (p > 30)
          n = 2;
        else
          n = 3;

        // assign output parameter dimension
        AllNodes[i][j]->MatDim[2] = rand() % 1000 + 40;

        // generate nodes and connection
        if (i < depth - 1) {
          for (int32_t k = 0; k < n; k++) {
            p = rand() % 100;
            int32_t jump;
            if (p > 50)
              jump = 1;
            else if (p > 30)
              jump = 2;
            else
              jump = 3;

            // will point from AllNodes[i][j] to AllNodes[i_end][j_end]
            int32_t i_end = i + jump;
            if (i_end >= depth && k == n - 1 && AllNodes[i][j]->NodesOut.empty()) i_end = depth - 1;
            if (i_end < depth) {
              int32_t j_end = rand() % AllNodes[i_end].size();
              if (AllNodes[i_end][j_end]) {
                // check if we had same edge before
                bool check_duplicated = false;
                for (const auto &node_out : AllNodes[i][j]->NodesOut) {
                  if (node_out == AllNodes[i_end][j_end]) { check_duplicated = true; }
                }
                if (check_duplicated) continue;
                // pass check and add data to exist node
                AllNodes[i_end][j_end]->MatDim[1] += AllNodes[i][j]->MatDim[2];
              } else {
                // create a new node for edge out
                AllNodes[i_end][j_end] =
                    new SbpNode<SbpSignature>(DataSize, AllNodes[i][j]->MatDim[2], 0);
              }
              AllNodes[i][j]->PointTo(AllNodes[i_end][j_end]);
            }
          }
        }
      }
    }
  }

  // Store all nodes
  for (int32_t i = 0; i < AllNodes.size(); i++) {
    for (int32_t j = 0; j < AllNodes[i].size(); j++) {
      if (AllNodes[i][j]) {
        NodeList.emplace_back(AllNodes[i][j]);
        AllNodes[i][j]->id = NextId++;
        AllNodes[i][j]->NodeListId = NodeList.size() - 1;
      }
    }
  }
  OriginalNodeList = NodeList;
};

#endif  // RANDOM_GENERATOR_

#ifdef DEBUG_ALGORITHM_

template<class SbpSignature>
double SbpGraph<SbpSignature>::ComputeOriginCost() {
  double GraphCost = 0;
  for (SbpNode<SbpSignature> *this_node : OriginalNodeList) {
    int32_t this_id = this_node->FinalSbpSignatureId;
    GraphCost += this_node->OriginCost[this_id];
    for (int32_t j = 0; j < this_node->OriginCostOut.size(); j++) {
      GraphCost +=
          this_node->OriginCostOut[j][this_id][this_node->NodesOut[j]->FinalSbpSignatureId];
    }
  }
  return GraphCost;
}

#endif  // DEBUG_ALGORITHM_

#ifdef PRINT_GRAPH_

template<class SbpSignature>
void PrintNode(SbpNode<SbpSignature> *this_node) {
  if (!this_node->Children.empty()) {
    printf("[%d", this_node->Children[0]->id);
    for (int32_t i = 1; i < this_node->Children.size(); i++) {
      printf(", %d", this_node->Children[i]->id);
    }
    printf("] ");
  }

  if (!this_node->HalfNode.empty())
    printf("(%d, %d) ", this_node->HalfNode[0]->id, this_node->HalfNode[1]->id);

  printf("%d -> ", this_node->id);

  for (const auto &edge_out : this_node->EdgesOut) { printf("%d, ", edge_out->EndNode->id); }
  printf("\n");
}

template<class SbpSignature>
void SbpGraph<SbpSignature>::PrintGraph() {
  // initialization for topological sorting
  InitTopologicalSort();

  // strict topological sorting
  std::vector<SbpNode<SbpSignature> *> TopoSortNode;
  for (const auto &this_node : NodeList) {
    if (this_node->CurrDeg == 0) TopoSortNode.emplace_back(this_node);
  }

  int32_t level = 0, level_start = 0, level_end = TopoSortNode.size();
  int32_t index = 0;
  while (index < TopoSortNode.size()) {
    // Move to next level
    if (index == level_end) {
      level++;
      level_start = level_end;
      level_end = TopoSortNode.size();
    }
    if (index == level_start) { printf("=================level:%d=================\n", level); }

    // Print each node and all out degrees
    PrintNode(TopoSortNode[index]);
    for (const auto &edge_out : TopoSortNode[index]->EdgesOut) {
      edge_out->EndNode->CurrDeg--;
      if (edge_out->EndNode->CurrDeg == 0) TopoSortNode.emplace_back(edge_out->EndNode);
    }

    index++;
  }

  printf("==============level:Circle===============\n");
  for (const auto &this_node : NodeList) {
    if (this_node->CurrDeg) PrintNode(this_node);
  }
  printf("\n|++++++++++++++++++++++++++++++++++|\n\n");
  // printf("\nCurrent Cost: %d, Origin Cost: %d\n\n", ComputeCost(), ComputeOriginCost());
};

template<class SbpSignature>
void SbpGraph<SbpSignature>::PrintSbpSigs() {
  printf("**********Sbp Signatures***********\n");
  for (const auto &this_node : OriginalNodeList) {
    printf("%d (Sbp: %d)\n", this_node->id,
           this_node->SbpSignatureList[this_node->FinalSbpSignatureId]->id);
  }
  printf("|*********************************|\n");
}

#endif  // PRINT_GRAPH_

}  // namespace Algorithm

#endif  // SBP_GRAPH_H_
