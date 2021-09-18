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
#ifndef SBP_EDGE_H_
#define SBP_EDGE_H_

//#include "SbpGraph.h"
#include "sbp_node.h"
#include <assert.h>
#include <algorithm>

#ifdef USE_SBP_COLLECTOR_
#include <unordered_set>
#include "oneflow/core/job/sbp_parallel.pb.h"
#include "oneflow/core/graph/op_graph.h"
#endif  // USE_SBP_COLLECTOR_

namespace Algorithm {

template<class SbpSignature>
class SbpEdge {
  /* There are 3 types of edges:
   * 1. StartNode -> EndNode
   *      Nothing special
   * 2. Multiple StartNode -> EndNode
   *      EdgeList will store all the edges which goes from StartNode to EndNode
   * 3. StartNode -> MidNode -> EndNode
   *      It will pass by a middle node.
   */
 public:
  // The edge point from StartNode to EndNode
  // It will have a middle node if and only if type 3
  SbpNode<SbpSignature> *StartNode, *MidNode, *EndNode;
  // Cost[sbp_i][sbp_j] is the total cost from StartNode with sbp_i to EndNode
  // with sbp_j
  std::vector<std::vector<double>> Cost;
  // SbpSignature for MidNode with corresponding Cost if type 3, empty otherwise
  std::vector<std::vector<int32_t>> MidNodeSbpSig;
  // Contained edge list:
  // empty if type 1,
  // Parallel edges if type 2,
  // succeed edges if type 3
  // the edge list might have reverse direction:
  // example 1: type 3 EdgeList contain two edges:
  //        MidNode -> StartNode, MidNode -> EndNode;
  // example 2: type 2 EdgeList contain three edges:
  //        StartNode -> EndNode, EndNode -> StartNode, StartNode -> EndNode;
  std::vector<SbpEdge<SbpSignature> *> EdgeList;

  // Mininum and maximum cost would not be changed by eliminations, which will generate new edges.
  // Also would not be changed by node merging, which will only perform cost copy for the expanding
  // dimensions.
  // Minimum cost in the 2D array Cost.
  // Would be initialized after GetMinCost();
  // Only used in the final graph.
  // double min_cost = -1.0;
  // Maximum cost in the 2D array Cost.
  // Would be initialized after GetMaxCost();
  // Only used in the original graph.
  // double max_cost = -1.0;
  // overlap ratio. Applied in copy cost.
  double overlap_ratio = 1.0;

 public:
  // Constructor for type 1 & 2
  SbpEdge(SbpNode<SbpSignature> *start_node, SbpNode<SbpSignature> *end_node)
      : StartNode(start_node), EndNode(end_node) {
    MidNode = NULL;
  }
  // Constructor for type 3
  SbpEdge(SbpNode<SbpSignature> *start_node, SbpNode<SbpSignature> *mid_node,
          SbpNode<SbpSignature> *end_node, SbpEdge<SbpSignature> *first_edge,
          SbpEdge<SbpSignature> *second_edge);
  // Compute copy cost for type 1
  void ComputeCost(
      const std::function<double(SbpNode<SbpSignature> *, SbpSignature *, SbpNode<SbpSignature> *,
                                 SbpSignature *)> &SbpInferHint4Ibn);

  // Update copy cost for type 2 and 3
  void SummerizeCost();
  // Duplicate Cost. Designed for merging two nodes.
  void DuplicateCost(bool ifStart, bool ifFirst,
                     const std::vector<std::pair<int32_t, int32_t>> &mergedSigId2ChildrenSigId);
  // Determine Final SbpSignature for attachment of this edge
  void FinalizeSbp();
  // Use Greedy Strategy to pick the sbp signature with minimum cost for this
  // edge. You should have an initial strategy before running this. And the
  // graph should be fully eliminated.
  double GreedyStrategy();
  // Deconstructor
  ~SbpEdge() {
    if (MidNode) { delete MidNode; }
    for (auto &this_edge : EdgeList) { delete this_edge; }
  }

  // a set of ids of logical blobs carried/transferred on this sbp edge
  std::unordered_set<oneflow::LogicalBlobId> CarryLbis;

  // load a logical blob
  void LoadLbi(oneflow::LogicalBlobId lbi) { CarryLbis.insert(lbi); }

  // check existancy of a logical blob
  bool SearchLbi(oneflow::LogicalBlobId lbi) const {
    return CarryLbis.find(lbi) != CarryLbis.end();
  }

  // unload a logical blob
  void UnloadLbi(oneflow::LogicalBlobId lbi) {
    if (CarryLbis.erase(lbi) == 0) std::cout << "Unload an empty lbi!" << std::endl;
  }

  // Not carrying any blob
  bool EmptyLbi() const { return CarryLbis.empty(); }

  // Get the minimum element in Cost
  double GetMinCost();
  // Get the maximum element in Cost
  double GetMaxCost();

  // Detect and spread overlaps for this edge and StartNode if it is a proxy of lbi.
  void DetectSpreadOverlap(double overlap_ratio_);

  // Adjust cost with overlaps
  void AdjustOverlapCost();
};
}  // namespace Algorithm
// function in cpp. Should be put in one file due to use of template
// Otherwise we will need to declare specific template at the end of cpp file.
namespace Algorithm {
template<class SbpSignature>
SbpEdge<SbpSignature>::SbpEdge(SbpNode<SbpSignature> *start_node, SbpNode<SbpSignature> *mid_node,
                               SbpNode<SbpSignature> *end_node, SbpEdge<SbpSignature> *first_edge,
                               SbpEdge<SbpSignature> *second_edge)
    : StartNode(start_node), MidNode(mid_node), EndNode(end_node) {
  EdgeList.emplace_back(first_edge);
  EdgeList.emplace_back(second_edge);
};

template<class SbpSignature>
void SbpEdge<SbpSignature>::ComputeCost(
    const std::function<double(SbpNode<SbpSignature> *, SbpSignature *, SbpNode<SbpSignature> *,
                               SbpSignature *)> &SbpInferHint4Ibn) {
  Cost.resize(StartNode->Cost.size());
  int32_t EndNodeSbpSize = EndNode->Cost.size();
  for (int32_t sbp_start = 0; sbp_start < Cost.size(); sbp_start++) {
    Cost[sbp_start].resize(EndNodeSbpSize);
    for (int32_t sbp_end = 0; sbp_end < EndNodeSbpSize; sbp_end++) {
      Cost[sbp_start][sbp_end] = SbpInferHint4Ibn(StartNode, StartNode->SbpSignatureList[sbp_start],
                                                  EndNode, EndNode->SbpSignatureList[sbp_end]);
    }
  }
};

template<class SbpSignature>
void SbpEdge<SbpSignature>::SummerizeCost() {
  if (MidNode) {
    Cost.resize(StartNode->Cost.size());
    MidNodeSbpSig.resize(StartNode->Cost.size());
    int32_t EndNodeSbpSize = EndNode->Cost.size();
    int32_t MidNodeSbpSize = MidNode->Cost.size();
    for (int32_t sbp_start = 0; sbp_start < Cost.size(); sbp_start++) {
      Cost[sbp_start].resize(EndNodeSbpSize);
      MidNodeSbpSig[sbp_start].resize(EndNodeSbpSize);
      for (int32_t sbp_end = 0; sbp_end < EndNodeSbpSize; sbp_end++) {
        for (int32_t sbp_mid = 0; sbp_mid < MidNodeSbpSize; sbp_mid++) {
          // Add middle node cost
          double temp = MidNode->Cost[sbp_mid];
          // Add first edge cost
          if (EdgeList[0]->StartNode == StartNode)
            temp += EdgeList[0]->Cost[sbp_start][sbp_mid];
          else
            temp += EdgeList[0]->Cost[sbp_mid][sbp_start];
          // Add second edge cost
          if (EdgeList[1]->EndNode == EndNode)
            temp += EdgeList[1]->Cost[sbp_mid][sbp_end];
          else
            temp += EdgeList[1]->Cost[sbp_end][sbp_mid];

          // Compare and look for the minimum cost
          if (sbp_mid == 0) {
            Cost[sbp_start][sbp_end] = temp;
            MidNodeSbpSig[sbp_start][sbp_end] = sbp_mid;
          } else if (temp < Cost[sbp_start][sbp_end]) {
            Cost[sbp_start][sbp_end] = temp;
            MidNodeSbpSig[sbp_start][sbp_end] = sbp_mid;
          }
        }
      }
    }
  } else {
    Cost.resize(StartNode->Cost.size());
    int32_t EndNodeSbpSize = EndNode->Cost.size();
    for (int32_t sbp_start = 0; sbp_start < Cost.size(); sbp_start++) {
      Cost[sbp_start].resize(EndNodeSbpSize);
      for (int32_t sbp_end = 0; sbp_end < EndNodeSbpSize; sbp_end++) {
        Cost[sbp_start][sbp_end] = 0;
        for (int32_t edge_num = 0; edge_num < EdgeList.size(); edge_num++) {
          if (EdgeList[edge_num]->StartNode == StartNode) {
            Cost[sbp_start][sbp_end] += EdgeList[edge_num]->Cost[sbp_start][sbp_end];
          } else {
            Cost[sbp_start][sbp_end] += EdgeList[edge_num]->Cost[sbp_end][sbp_start];
          }
        }
      }
    }
  }
}

template<class SbpSignature>
void SbpEdge<SbpSignature>::DuplicateCost(
    bool ifStart, bool ifFirst,
    const std::vector<std::pair<int32_t, int32_t>> &mergedSigId2ChildrenSigId) {
  const int32_t num_sig = mergedSigId2ChildrenSigId.size();
  std::vector<std::vector<double>> tmpCost;
  std::vector<std::vector<int32_t>> tmpMidNodeSbpSig;
  if (ifStart) {
    tmpCost.resize(num_sig);
    if (MidNode) tmpMidNodeSbpSig.resize(num_sig);
    for (int32_t i = 0; i < num_sig; i++) {
      const int32_t sig_idx =
          ifFirst ? mergedSigId2ChildrenSigId[i].first : mergedSigId2ChildrenSigId[i].second;
      tmpCost[i] = Cost[sig_idx];
      if (MidNode) tmpMidNodeSbpSig[i] = MidNodeSbpSig[sig_idx];
    }
  } else {
    const int32_t num_start_sig = Cost.size();
    tmpCost.resize(num_start_sig);
    if (MidNode) tmpMidNodeSbpSig.resize(num_start_sig);
    for (int32_t i = 0; i < num_start_sig; i++) {
      tmpCost[i].resize(num_sig);
      if (MidNode) tmpMidNodeSbpSig[i].resize(num_sig);
      for (int32_t j = 0; j < num_sig; j++) {
        const int32_t sig_idx =
            ifFirst ? mergedSigId2ChildrenSigId[j].first : mergedSigId2ChildrenSigId[j].second;
        tmpCost[i][j] = Cost[i][sig_idx];
        if (MidNode) tmpMidNodeSbpSig[i][j] = MidNodeSbpSig[i][sig_idx];
      }
    }
  }

  Cost = tmpCost;
  if (MidNode) MidNodeSbpSig = tmpMidNodeSbpSig;
}

template<class SbpSignature>
void SbpEdge<SbpSignature>::FinalizeSbp() {
#ifdef DEBUG_ALGORITHM_
  // printf("Finalize Sbp edge: from %d to %d, EdgeListNum: %d\n",
  // StartNode->id,
  //       EndNode->id, EdgeList.size());
#endif  // DEBUG_ALGORITHM_

  // Finalize Sbp for MidNode
  if (MidNode) {
    MidNode->FinalSbpSignatureId =
        MidNodeSbpSig[StartNode->FinalSbpSignatureId][EndNode->FinalSbpSignatureId];
    MidNode->FinalizeSbp();
  }
  for (const auto &this_edge : EdgeList) { this_edge->FinalizeSbp(); }
}

template<class SbpSignature>
double SbpEdge<SbpSignature>::GreedyStrategy() {
  // Sbp combination of the minimum cost
  int32_t MinSbpStart = StartNode->FinalSbpSignatureId, MinSbpEnd = EndNode->FinalSbpSignatureId;
  // An unordered_map to evaluate cost between two edge nodes and other nodes.
  std::unordered_map<int32_t, int32_t> EdgeNodeListIds = {{StartNode->NodeListId, 0},
                                                          {EndNode->NodeListId, 1}};
  // pre-compute and store the Current Cost for EndNode.
  std::vector<double> EndNodeCurrCost(EndNode->Cost.size());
  for (int32_t sbp_end = 0; sbp_end < Cost[0].size(); sbp_end++) {
    EndNode->FinalSbpSignatureId = sbp_end;
    EndNodeCurrCost[sbp_end] = EndNode->EvalOutNbhCost(EdgeNodeListIds);
  }
  // pre-copmpute and store the Current Cost for StartNode.
  std::vector<double> StartNodeCurrCost(StartNode->Cost.size());
  for (int32_t sbp_start = 0; sbp_start < Cost.size(); sbp_start++) {
    StartNode->FinalSbpSignatureId = sbp_start;
    StartNodeCurrCost[sbp_start] = StartNode->EvalOutNbhCost(EdgeNodeListIds);
  }
  // Current Cost, Minimum Cost, Cost with original sbp
  double CurrCost, MinCost, OrgCost;
  MinCost =
      StartNodeCurrCost[MinSbpStart] + EndNodeCurrCost[MinSbpEnd] + Cost[MinSbpStart][MinSbpEnd];
  OrgCost = MinCost;

  for (int32_t sbp_start = 0; sbp_start < Cost.size(); sbp_start++) {
    for (int32_t sbp_end = 0; sbp_end < Cost[0].size(); sbp_end++) {
      // compute Current Cost for Neighborhood of edge
      EndNode->FinalSbpSignatureId = sbp_end;
      CurrCost = StartNodeCurrCost[sbp_start] + EndNodeCurrCost[sbp_end] + Cost[sbp_start][sbp_end];
      // Find the minimum current cost
      if (CurrCost < MinCost) {
        MinCost = CurrCost;
        MinSbpStart = sbp_start;
        MinSbpEnd = sbp_end;
      }
    }
  }
  StartNode->FinalSbpSignatureId = MinSbpStart;
  EndNode->FinalSbpSignatureId = MinSbpEnd;
  return MinCost - OrgCost;
}

// Get the minimum element in Cost
template<class SbpSignature>
double SbpEdge<SbpSignature>::GetMinCost() {
  // used the stored value if pre-computed.
  // if (min_cost >= 0) return min_cost;
  // Check the size of Cost
  CHECK(Cost.size() > 0) << "Cost not initialized!" << std::endl;
  // Compute the min_cost
  double min_cost = *std::min_element(Cost[0].begin(), Cost[0].end());
  for (int32_t i = 1; i < Cost.size(); i++) {
    double min_cost_row = *std::min_element(Cost[i].begin(), Cost[i].end());
    if (min_cost_row < min_cost) min_cost = min_cost_row;
  }
  return min_cost;
}

// Get the maximum element in Cost
template<class SbpSignature>
double SbpEdge<SbpSignature>::GetMaxCost() {
  // used the stored value if pre-computed.
  // if (max_cost >= 0) return max_cost;
  // Check the size of Cost
  CHECK(Cost.size() > 0) << "Cost not initialized!" << std::endl;
  // Compute the max_cost
  double max_cost = -1.0;
  for (int32_t i = 0; i < Cost.size(); i++) {
    for (int32_t j = 0; j < Cost[i].size(); j++){
      if(Cost[i][j] < 1e38 && Cost[i][j] > max_cost) max_cost = Cost[i][j];
    }
  }
  return max_cost;
}

// Detect and spread overlaps for this edge and StartNode if it is a proxy of lbi.
template<class SbpSignature>
void SbpEdge<SbpSignature>::DetectSpreadOverlap(double overlap_ratio_) {
  // test debug
  std::cout << "StartNode: ";
  if (StartNode->op_node) {
    std::cout << StartNode->op_node->op().op_name();
  } else {
    std::cout << "proxy";
  }
  std::cout << " to ";
  if (EndNode->op_node) {
    std::cout << EndNode->op_node->op().op_name();
  } else {
    std::cout << "proxy";
  }
  std::cout << ", overlap ratio: " << overlap_ratio_ << std::endl;

  if (overlap_ratio_ < 1.0) {
    if (overlap_ratio_ < 0.0) overlap_ratio_ = 0.0;

    if (StartNode->op_node) {
      // change overlap ratio for a normal edge
      // We could use the minimum or multiplication here.
      // To be noted that sbp_proxy may have multiple outcoming edges, we can not adjust overlap
      // cost right away.
      if (overlap_ratio_ < overlap_ratio) overlap_ratio = overlap_ratio_;
    } else {
      // For an edge with a proxy start node, do not change the overlap ratio since the cost only
      // contains 0 and 3e38. Change the overlap ratio for the previous edge.
      StartNode->DetectSpreadOverlap(overlap_ratio_);
    }
  }
}

// Adjust cost with overlaps
template<class SbpSignature>
void SbpEdge<SbpSignature>::AdjustOverlapCost() {
  if (overlap_ratio >= 1.0) return;
  if (overlap_ratio < 0.0) overlap_ratio = 0.0;
  for (int32_t i = 0; i < Cost.size(); i++) {
    for (int32_t j = 0; j < Cost[i].size(); j++) {
      if (Cost[i][j] > 0.0 && Cost[i][j] < 1e38) { Cost[i][j] = overlap_ratio * Cost[i][j]; }
    }
  }
}

}  // namespace Algorithm

#endif  // SBP_EDGE_H_