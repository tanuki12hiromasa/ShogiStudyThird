#include "josekioutput.h"
#include <fstream>

JosekiOutput::JosekiOutput(){
	option.addOption("outputFileName", "string", "joseki\\defaultjoseki_output.bin");
	option.addOption("outputFileInfoName", "string", "joseki\\defaultjoseki_output_info.txt");
	option.addOption("backup_T_e", "string", "40");
	option.addOption("backup_T_d", "string", "80");
}

//定跡書き出し
void JosekiOutput::josekiOutput(const std::vector<SearchNode*> const history) {
	if (!outputInfo(history)) {
		return;
	}

	backUp(history);

	//ここからファイルに書き出し
	size_t index = 0;
	size_t childIndex = 1;
	size_t nodeCount = SearchNode::getNodeCount();

	SearchNode** nodes = (SearchNode**)malloc(sizeof(SearchNode*) * nodeCount);
	josekinode* jn;
	jn = (josekinode*)malloc(sizeof(josekinode) * nodeCount);

	nodes[0] = history.front();
	for (index = 0; index < nodeCount; ++index) {
		SearchNode* node = nodes[index];	//nodesから注目ノードを取り出し
		const size_t childCount = node->children.size();
		SearchNode::State state = node->getState();
		if (childCount > 0) {
			state = SearchNode::State::Expanded;
		}
		jn[index] = josekinode(index, state, node->move.getU(), node->mass, node->getEvaluation(), childCount, childIndex);	//注目ノードをjnに収める

		for (int i = 0; i < childCount; ++i) {	//子ノードをnodesに格納
			nodes[childIndex + i] = &(node->children[i]);
		}

		childIndex += childCount;	//子ノードの数だけchildIndexを進める
	}

	//書き出しファイルオープン
	FILE* fp;
	fopen_s(&fp, option.getS("outputFileName").c_str(), "wb");
	fwrite(jn, sizeof(jn[0]), nodeCount, fp);	//一気に書き出し
	fflush(fp);
	fclose(fp);

	free(jn);
	/*for (int i = 0; i < nodeCount; ++i) {
		free(nodes[i]);
	}*/
	free(nodes);

}

bool JosekiOutput::outputInfo(const std::vector<SearchNode*> const history)
{
	std::string moveHis = "";
	std::string usiHis = "";
	for (SearchNode* his : history) {
		moveHis += std::to_string(his->move.getU());
		moveHis += ",";
		if (his->move.toUSI() == "nullmove" || his->move.toUSI() == "1a1a") {
			usiHis += "position startpos moves";
		}
		else {
			usiHis += his->move.toUSI();
		}
		usiHis += " ";
	}

	//書き出しノード保存用キュー
	std::queue<SearchNode*> nq;
	nq.push(history.front());

	//ノードの数を数え、infoファイルに出力する
	//std::cout << "ソート完了" << std::endl;
	size_t nodeCount = SearchNode::getNodeCount();

	std::ofstream ofs(option.getS("outputFileInfoName"));
	ofs << "nodeCount," << nodeCount << std::endl;
	ofs << moveHis << std::endl;
	ofs << usiHis << std::endl;
	ofs << "depth," << nq.front()->mass << std::endl;

	//ノードの数が多すぎるとメモリの限界を超えるため、出力を中止する
	ofs << "nodesize" << "," << sizeof(josekinode) << std::endl;
	size_t fileSize = nodeCount * sizeof(josekinode);
	size_t gigabyte = 1024 * 1024 * 1024;
	size_t maxByte = 10 * gigabyte;
	ofs << "推定ファイルサイズ：" << std::to_string(fileSize) << "バイト(" << (double)fileSize / gigabyte << "ギガバイト)" << std::endl;
	std::cout << "推定ファイルサイズ：" << std::to_string(fileSize) << "バイト" << std::endl;
	std::cout << "推定ファイルサイズ：" << std::to_string((double)fileSize / gigabyte) << "ギガバイト" << std::endl;
	if (fileSize >= maxByte) {
		std::cout << "書き出そうとしているファイルサイズが大きすぎます。" << std::endl;
		std::cout << "最大サイズ：" << maxByte << std::endl;
		std::cout << "出力を中止します。" << std::endl;
		ofs.close();
		return false;
	}
	return true;
}


void JosekiOutput::backUp(std::vector<SearchNode*> history)
{
	std::cout << "バックアップを行います。" << std::endl;
	const double MateScoreBound = 30000.0;
	typedef std::pair<double, double> dd;
	double T_e = option.getD("backup_T_e");
	double T_d = option.getD("backup_T_d");
	SearchNode* node = history.back();

	for (int i = history.size() - 2; i >= 0; i--) {
		node = history[i];
		double emin = 99999;
		std::vector<dd> emvec;
		for (const auto& child : node->children) {
			const double eval = child.getEvaluation();
			const double mass = child.mass;
			emvec.push_back(std::make_pair(eval, mass));
			if (eval < emin) {
				emin = eval;
			}
		}
		if (std::abs(emin) > MateScoreBound) {
			node->setMateVariation(emin);
		}
		else {
			double Z_e = 0;
			double Z_d = 0;
			for (const auto& em : emvec) {
				const double eval = em.first;
				Z_e += std::exp(-(eval - emin) / T_e);
				Z_d += std::exp(-(eval - emin) / T_d);
			}
			double E = 0;
			double M = 1;
			for (const auto& em : emvec) {
				const double eval = em.first;
				const double mass = em.second;
				E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
				M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
			}

			//std::cout << "[" << i << "](" << node->eval << "→";
			//node->setEvaluation(E * (i == history.size() - 2 ? (double)i * i : 1));
			node->setEvaluation(E);
			node->setMass(M);
			//std::cout << node->eval << ") ";
		}
	}
	std::cout << std::endl;
	std::cout << "バックアップ完了" << std::endl;
}

