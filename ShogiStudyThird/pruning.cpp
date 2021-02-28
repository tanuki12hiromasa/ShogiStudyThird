#include "pruning.h"
#include <iostream>

size_t Pruning::pruning(SearchNode* root) {
	size_t r = 0;

	std::cout << "枝刈りを行います" << std::endl;

	std::vector<SearchNode*>history;
	r = partialPruning(root, history, 1, 0);

	return r;
}

size_t Pruning::partialPruning(SearchNode* node, std::vector<SearchNode*> history, double select, int depth, double backupRate) {
	size_t r = 0;
	//末端ならその場で終了
	if (node->children.size() == 0) {
		return r;
	}
	double mass = node->mass;
	history.push_back(node);
	//枝刈り判定を行う
	if (isPruning(node, select, depth, backupRate)) {
		r += pruningExecuter(node, history);
	}
	else {
		if (pruning_type >= 0) {
			//実現確率の計算
			if (select != -1) {
				double CE = node->children[0].getEvaluation();
				//評価値の最大値の取得
				for (SearchNode& child : node->children) {
					if (CE < child.getEvaluation()) {
						CE = child.getEvaluation();
					}
				}
				//バックアップ温度
				double T_c = pruning_T_c;
				if (pruning_type == 1) {
					T_c += 10000 * pow(0.1, depth);
				}
				else if (pruning_type == 2) {
					T_c += 1 * pow(2, depth);
				}
				double Z = 0;
				for (SearchNode& child : node->children) {
					Z += std::exp(-(child.getEvaluation() - CE) / T_c);
				}
				for (int i = 0; i < node->children.size(); ++i) {
					//再帰的に枝刈りを行う
					SearchNode* child = &node->children[i];
					double s;
					if (leaveNodeCount == 0 || i < leaveNodeCount) {
						s = std::exp(-(child->getEvaluation() - CE) / T_c) / Z;
					}
					else {
						s = 0;
					}
					r += partialPruning(child, history, s * select, depth + 1);
				}
			}
			else {
				//再帰的に枝刈りを行う
				for (int i = 0; i < node->children.size();++i) {
					r += partialPruning(&node->children[i], history);
				}
			}
		}

	}
	return r;
}

size_t Pruning::pruningExecuter(SearchNode* node, std::vector<SearchNode*> history) {
	node->children.clear();
	if (node->getState() != SearchNode::State::Terminal) {
		node->restoreNode(node->move,SearchNode::State::NotExpanded,node->eval,node->mass);
	}
	return 0;
}

bool Pruning::isPruning(SearchNode* node, double select, int depth, double backupRate) {
	//実現確率がpruningBorder%以下なら切り捨て
	switch (pruning_type)
	{
	case 0:
	default:
		//%で考えてるから0.01倍
		if (select < pruningBorder * 0.01) {
			return true;
		}
		break;

	}
	return false;
}
