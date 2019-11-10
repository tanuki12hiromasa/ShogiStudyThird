#include "mate_checker.h"

void MateChecker::children(Kyokumen pkyokumen, std::vector<SearchNode*> children) {
	if (pkyokumen.teban()) {
		//子ノードは後手
		for (auto& node : children) {
			Kyokumen kyokumen = pkyokumen;
			kyokumen.proceed(node->move);
			const auto kusemono = kyokumen.getGoteOuCheck(node->move);
			if (!kusemono.empty()) {
				node->move.setOute(true);
				if (kyokumen.isGoteMate(kusemono)) {
					node->setMate();
				}
			}
		}
	}
	else {
		//子ノードは先手
		for (auto& node : children) {
			Kyokumen kyokumen = pkyokumen;
			kyokumen.proceed(node->move);
			const auto kusemono = kyokumen.getSenteOuCheck(node->move);
			if (!kusemono.empty()) {
				node->move.setOute(true);
				if (kyokumen.isSenteMate(kusemono)) {
					node->setMate();
				}
			}
		}
	}
}