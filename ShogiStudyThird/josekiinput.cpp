#include "josekiinput.h"
#include <fstream>
#include "usi.h"

JosekiInput::JosekiInput(){
	option.addOption("joseki_input_on", "check", "false");
	option.addOption("joseki_input_folder_name", "string", "joseki");
	option.addOption("joseki_input_file_name", "string", "defaultjoseki_input.bin");
	option.addOption("joseki_input_file_info_name", "string", "defaultjoseki_input_info.txt");
	option.addOption("joseki_sokusashi_on", "check", "false");
	option.addOption("joseki_sokusashi_border", "string", "5.0");
	bestMoveOn = true;
	dummy->restoreNode(Move(0), SearchNode::State::T, 0, -1);

}

void JosekiInput::init() {
	if (!option.getC("joseki_input_on")) {
		return;
	}


	//定跡の情報が入ったファイルを開く
	std::ifstream ifs(option.getS("joseki_input_folder_name") + "\\" + option.getS("joseki_input_file_info_name"));
	if (!ifs.is_open()) {
		std::cout << option.getS("joseki_input_folder_name") + "\\" + option.getS("joseki_input_file_info_name") << "が開けませんでした。" << std::endl;
		std::cout << "start without joseki" << std::endl;
		return;
		//exit(EXIT_FAILURE);
	}

	//ノード数の取得
	std::string nodeCountStr;
	std::getline(ifs, nodeCountStr);
	std::int64_t nodeCount = std::stol(usi::split(nodeCountStr, ',')[1]);    //infoファイルからノード数を取得
	SearchNode::setNodeCount(nodeCount);

	//positionの取得
	std::string pos;
	std::getline(ifs, pos); //moveが保存された行
	std::getline(ifs, pos); //positionが保存された行
	//tokenOfPosition = usi::split(pos, ' ');
	tokenOfPosition.push_back("position");
	tokenOfPosition.push_back("startpos");
	tokenOfPosition.push_back("moves");

	ifs.close();

	std::cout << "node num:" << nodeCount << std::endl;

	//定跡本体を開く
	errno_t err = fopen_s(&fp, (option.getS("joseki_input_folder_name") + "\\" + option.getS("joseki_input_file_name")).c_str(), "rb");
	if (err) {
		std::cout << option.getS("joseki_input_folder_name") + "\\" + option.getS("joseki_input_file_name") << "が開けませんでした。" << std::endl;
		exit(EXIT_FAILURE);
	}
	nodesFromFile = (josekinode*)calloc(nodeCount, sizeof(josekinode));	//定跡の復元に一時的に利用するノード
	fread(nodesFromFile, sizeof(josekinode), nodeCount, fp);	//定跡本体をバイナリファイルから読み込み

	//nodesForProgram = (SearchNode*)calloc(nodeCount, sizeof(SearchNode));	//プログラム内で使用するnode
	nodesForProgram = new SearchNode[nodeCount];
	parentsIndex = (size_t*)calloc(nodeCount, sizeof(size_t));
}

void JosekiInput::josekiInput(SearchTree* tree, size_t firstIndex) {
	if (!option.getC("joseki_input_on")) {
		return;
	}


	//すぐ下の子ノード達だけ展開する
	int depth = 0;
	std::vector<size_t>childrenToThread = yomikomiDepth(firstIndex, depth);
	if (childrenToThread.size() <= 0) {
		std::cout << "子ノードがありません。" << std::endl;
		std::cout << "プログラムを強制終了します。" << std::endl;
		exit(EXIT_FAILURE);
	}

	//子ノードの数だけ再帰的に読みこむ
	std::vector<std::thread> thr;
	for (auto c : childrenToThread) {
		thr.push_back(std::thread(&JosekiInput::yomikomiRecursive, this, c));
	}
	std::cout << "thread num:" << thr.size() << std::endl;
	for (int i = 0; i < thr.size(); ++i) {
		thr[i].join();
	}

	SearchNode* list = new SearchNode[childrenToThread.size()];
	for (int i = 0; i < childrenToThread.size(); ++i) {
		SearchNode* cc = &nodesForProgram[childrenToThread[i]];
		list[i].restoreNode(cc->move, cc->getState(), cc->eval, cc->mass);
	}
	nodesForProgram[firstIndex].children.setChildren(list, childrenToThread.size());


	auto nextRoot = &nodesForProgram[firstIndex];
	//std::cout << "info next pv " << nextRoot->move.toUSI() << " cp " << nextRoot->eval << " depth " << nextRoot->mass << std::endl;
	tree->setRoot(nextRoot);

	free(nodesFromFile);
	free(parentsIndex);
	//free(nodesForProgram);
	fclose(fp);
}

//定跡バイナリファイルから最善手を取り出す
bool JosekiInput::getBestMove(SearchTree* tree, std::vector<SearchNode*> history)
{
	if (bestMoveOn == false) {
		return false;
	}

	if (option.getC("joseki_sokusashi_on") == false) {
		bestMoveOn = false;
		return false;
	}

	FILE* fp;
	fopen_s(&fp, (option.getS("joseki_input_folder_name") + "\\" + option.getS("joseki_input_file_name")).c_str(), "rb");
	size_t index = 0;

	josekinode jn;
	fseek(fp, index * sizeof(josekinode), SEEK_SET);
	fread_s(&jn, sizeof(josekinode), sizeof(josekinode), 1, fp);
	index = jn.childIndex;
	size_t lastIndex = 0;

	for (int i = 1; i < history.size() + 1;++i) {
		if (i == history.size()) {
			fseek(fp, index * sizeof(josekinode), SEEK_SET);
			fread_s(&jn, sizeof(josekinode), sizeof(josekinode), 1, fp);
			//SearchNode* r = new SearchNode;
			//r->restoreNode(Move(jn.move), jn.st, jn.eval, jn.mass);
			//std::cout << "info getbestmove pv " << Move(jn.move).toUSI() << " depth " << jn.mass << " score cp " << jn.eval << std::endl;
			if (jn.mass >= option.getD("joseki_sokusashi_border")) {
				std::cout << "bestmove " << Move(jn.move).toUSI() << std::endl;
				fclose(fp);
				return true;
			}
			else {
				break;
			}
		}

		fseek(fp, index * sizeof(josekinode), SEEK_SET);
		lastIndex = index;
		for (int j = 0; j < jn.childCount; ++j) {
			fread_s(&jn, sizeof(josekinode), sizeof(josekinode), 1, fp);
			if (jn.move == history[i]->move.getU()) {
				if (jn.childCount != 0) {
					index = jn.childIndex;
				}
				break;
			}
		}
		if (lastIndex == index) {
			break;
		}
	}
	bestMoveOn = false;

	josekiInput(tree, lastIndex);
	fclose(fp);
	return false;
}

static std::mutex mtx;
//1行読みこむ。fpをそのまま渡す用
std::vector<size_t> JosekiInput::yomikomiLine(const size_t index) {
	std::vector<size_t> childIndexes;
	josekinode node = nodesFromFile[index];


	size_t tIndex = index;

	Move move = Move(node.move);

	childIndexes.reserve(node.childCount);
	for (int i = 0; i < node.childCount; ++i) {		//子ノードのインデックスが読み終わるまでループ
		childIndexes.push_back(node.childIndex + i);
		parentsIndex[childIndexes.back()] = tIndex;	 //親のインデックスを要素として持つ
	}

	nodesForProgram[tIndex].restoreNode(move, node.st, node.eval, node.mass);

	return childIndexes;
}

//indexとその子供を再帰的に読みこむ
void JosekiInput::yomikomiRecursive(const size_t index) {
	auto children = yomikomiLine(index);
	SearchNode* list = new SearchNode[children.size()];
	for (int i = 0; i < children.size();++i) {
		yomikomiRecursive(children[i]);
		SearchNode* c = &nodesForProgram[children[i]];
		list[i].restoreNode(c->move, c->getState(), c->eval, c->mass);
	}
	nodesForProgram[index].children.setChildren(list, children.size());
}

//指定された深さまで読みこみ、子供たちを返す。0で指定されたindexのみ。並列処理はしない
std::vector<size_t> JosekiInput::yomikomiDepth(const size_t index, const int depth) {
	auto c = yomikomiLine(index);



	if (depth == 0) {
		return c;
	}
	else {
		std::vector<size_t>children;
		for (int i = 0; i < c.size();++i) {
			auto v = yomikomiDepth(c[i], depth - 1);
			children.insert(children.end(), v.begin(), v.end());
		}
		return children;
	}
}

