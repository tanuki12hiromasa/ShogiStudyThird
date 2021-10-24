#include "stdafx.h"
#include "kyokumen.h"
#include "usi.h"
#include "bb_kiki.h"

using namespace koma;

const std::array<std::uint8_t, 95> prime_bammen = {
	static_cast<std::uint8_t>(Koma::g_Kyou),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Kyou),
	static_cast<std::uint8_t>(Koma::g_Kei),static_cast<std::uint8_t>(Koma::g_Kaku),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::s_Hi),static_cast<std::uint8_t>(Koma::s_Kei),
	static_cast<std::uint8_t>(Koma::g_Gin),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Gin),
	static_cast<std::uint8_t>(Koma::g_Kin),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Kin),
	static_cast<std::uint8_t>(Koma::g_Ou),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Ou),
	static_cast<std::uint8_t>(Koma::g_Kin),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Kin),
	static_cast<std::uint8_t>(Koma::g_Gin),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Gin),
	static_cast<std::uint8_t>(Koma::g_Kei),static_cast<std::uint8_t>(Koma::g_Hi),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::s_Kaku),static_cast<std::uint8_t>(Koma::s_Kei),
	static_cast<std::uint8_t>(Koma::g_Kyou),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Kyou),
	0u,0u,0u,0u,0u,0u,0u,
	0u,0u,0u,0u,0u,0u,0u
};

const std::array<Koma, 11> sStepKomas = { Koma::s_Fu, Koma::s_Kei, Koma::s_Gin, Koma::s_Kin, Koma::s_Ou, Koma::s_nFu, Koma::s_nKyou, Koma::s_nKei, Koma::s_nGin, Koma::s_nKaku, Koma::s_nHi };
const std::array<Koma, 5> sDashKomas = { Koma::s_Kyou, Koma::s_Kaku, Koma::s_Hi, Koma::s_nKaku, Koma::s_nHi };
const std::array<Koma, 11> gStepKomas = { Koma::g_Fu, Koma::g_Kei, Koma::g_Gin, Koma::g_Kin, Koma::g_Ou, Koma::g_nFu, Koma::g_nKyou, Koma::g_nKei, Koma::g_nGin, Koma::g_nKaku, Koma::g_nHi };
const std::array<Koma, 5> gDashKomas = { Koma::g_Kyou, Koma::g_Kaku, Koma::g_Hi, Koma::g_nKaku, Koma::g_nHi };

Kyokumen::Kyokumen():
	Kyokumen(prime_bammen,true){}

Kyokumen::Kyokumen(const std::array<koma::Koma, 81>& ban, const std::array<unsigned, 7>& smochi, const std::array<unsigned, 7>& gmochi,bool teban)
	: bammen({0}), isSente(teban)
{
	for (size_t i = 0; i < ban.size(); i++) {
		bammen[i] = static_cast<uint8_t>(ban[i]);
	}
	for (size_t i = 0; i < smochi.size(); i++) {
		bammen[i + 81ull] = static_cast<uint8_t>(smochi[i]);
	}
	for (size_t i = 0; i < gmochi.size(); i++) {
		bammen[i + 88ull] = static_cast<uint8_t>(gmochi[i]);
	}
	reflectBitboard();
}

Kyokumen::Kyokumen(const Bammen& bammen, bool teban)
	:bammen(bammen), isSente(teban)
{
	reflectBitboard();
}

Kyokumen::Kyokumen(Bammen&& bammen, bool teban)
	:bammen(std::move(bammen)),isSente(teban)
{
	reflectBitboard();
}

Kyokumen::Kyokumen(const std::vector<std::string>& tokens) {
	if (tokens[1] == "startpos") {
		bammen = prime_bammen;
		isSente = true;
	}
	else {
		assert(tokens[1] == "sfen");
		//盤面
		std::vector<std::string> usiban(usi::split(tokens[2], '/'));
		int dan = 0;
		for (auto& line : usiban) {
			int suji = 8;
			for (auto c = line.begin(), end = line.end(); c != end; ++c) {
				if ('0' < *c&&* c <= '9') {
					for (int num = *c - '0'; num > 0; --num) {
						setKoma(XYtoPos(suji, dan), Koma::None);
						--suji;
					}
				}
				else {
					if (*c == '+') {
						++c;
						setKoma(XYtoPos(suji, dan), promote(usi::usiToKoma(*c)));
					}
					else {
						setKoma(XYtoPos(suji, dan), usi::usiToKoma(*c));
					}
					--suji;
				}
			}
			++dan;
		}
		//手番
		isSente = (tokens[3] == "b");
		//持ち駒
		for (size_t t = static_cast<size_t>(Position::SQm_Min); t <= static_cast<size_t>(Position::SQm_Max); t++) {
			bammen[t] = 0;
		}
		auto mStr = tokens[4];
		if (mStr.front() != '-') {
			for (auto c = mStr.begin(), end = mStr.end(); c != end; ++c) {
				unsigned num = 1;
				if (*c == '1') {
					c++;
					num = *c - '0' + 10u;
					c++;
				}
				else if ('1' < *c && *c <= '9') {
					num = *c - '0';
					c++;
				}
				bammen[MochiToMpos(usi::usiToMochi(*c), std::isupper(*c))] = num;
			}
		}
	}
	reflectBitboard();
}

std::string Kyokumen::toSfen()const {
	std::string usi = "sfen ";
	//sq
	for (int y = 0; y < 9; y++) {
		int nonecount = 0;
		for (int x = 8; x >= 0; x--) {
			if (bammen[x * 9ull + y] != static_cast<std::uint8_t>(Koma::None)) {
				if (nonecount != 0) {
					usi += std::to_string(nonecount);
					nonecount = 0;
				}
				usi += usi::komaToUsi(getKoma(static_cast<Position>(x * 9 + y)));
			}
			else {
				nonecount++;
			}
		}
		if (nonecount != 0) {
			usi += std::to_string(nonecount);
		}
		if (y < 8)usi += '/';
	}
	usi += ' ';
	usi += usi::tebanToUsi(teban());
	usi += ' ';
	//mochi
	bool wrote_mochi = false;
	for (auto mpos : { Position::m_sHi,Position::m_sKaku,Position::m_sKin,
		Position::m_sGin,Position::m_sKei,Position::m_sKyou,Position::m_sFu,
		Position::m_gHi,Position::m_gKaku,Position::m_gKin,
		Position::m_gGin,Position::m_gKei,Position::m_gKyou,Position::m_gFu }) 
	{
		auto num = bammen[mpos];
		if (num > 0) {
			if (num > 1) {
				usi += std::to_string(num);
			}
			usi += usi::mposToUsi(mpos);
			wrote_mochi = true;
		}
	}
	if (!wrote_mochi) {
		usi += '-';
	}
	usi += " 1 ";
	return usi;
}

koma::Koma Kyokumen::proceed(const Move move) {
	const unsigned from = move.from(), to = move.to();
	const bool prom = move.promote();
	const Koma captured = getKoma(to);
	//sente,gote,allBB
	if (teban()) { //先手
		if (koma::isInside(from)) { //元位置が盤内ならそのビットを消す
			senteKomaBB.reset(from);
		}
		senteKomaBB.set(to); //行き先のビットを立てる
		goteKomaBB.reset(to); //行き先の敵盤ビットを下げる
	}
	else { //後手
		if (koma::isInside(from)) {
			goteKomaBB.reset(from);
		}
		goteKomaBB.set(to);
		senteKomaBB.reset(to);
	} //全体
	allKomaBB = senteKomaBB | goteKomaBB;

	//eachKomaBB,bammen
	if (koma::isInside(from)) {//盤上の駒を動かす場合
		const Koma fromKoma = getKoma(from);
		Bitboard& fromKomaBB = eachKomaBB[static_cast<size_t>(fromKoma)];
		fromKomaBB.reset(from);	//BB from
		bammen[from] = static_cast<std::uint8_t>(Koma::None);	//ban from
		if (prom) {//成る場合
			Koma fromPromKoma = promote(fromKoma);
			Bitboard& fromPromKomaBB = eachKomaBB[static_cast<size_t>(fromPromKoma)];
			fromPromKomaBB.set(to);	//BB to
			bammen[to] = static_cast<std::uint8_t>(fromPromKoma);//ban to
		}
		else {//成らない場合
			fromKomaBB.set(to);		//BB to
			bammen[to] = static_cast<std::uint8_t>(fromKoma);	//ban to
		}
		if (captured != Koma::None) {//駒を取っていた場合 
			Bitboard& toKomaBB = eachKomaBB[static_cast<size_t>(captured)];
			toKomaBB.reset(to);//BB cap
			bammen[KomaToMpos(captured)]++;//mban cap
			//bammen[to]は後で更新される
		}
	}
	else {//駒台から打つ場合
		const Koma fromKoma = MposToKoma(static_cast<Position>(from));
		Bitboard& fromKomaBB = eachKomaBB[static_cast<size_t>(fromKoma)];
		fromKomaBB.set(to);		//BB to
		bammen[from]--;		//mban from
		bammen[to] = static_cast<std::uint8_t>(fromKoma);	//ban to
	}
	isSente = !isSente;
	return captured;
}

koma::Koma Kyokumen::recede(const Move move, const koma::Koma captured) {
	using namespace koma;
	const unsigned from = move.from(), to = move.to();
	const bool prom = move.promote();
	const Koma fromKoma = getKoma(to);
	//手番を戻す
	isSente = !isSente;
	//senteBB,goteBB
	if (teban()) {
		if (captured != Koma::None) {
			goteKomaBB.set(to);
		}
		senteKomaBB.reset(to);
		if (isInside(from)) {
			senteKomaBB.set(from);
		}
	}
	else {
		if (captured != Koma::None) {
			senteKomaBB.set(to);
		}
		goteKomaBB.reset(to);
		if (isInside(from)) {
			goteKomaBB.set(from);
		}
	}
	allKomaBB = senteKomaBB | goteKomaBB;

	//eachBB,bammen
	if (koma::isInside(move.from())) {
		Bitboard& fromKomaBB = eachKomaBB[static_cast<size_t>(fromKoma)];
		fromKomaBB.reset(to);
		if (prom) {//成った場合,成りを戻す
			const Koma fromDispromKoma = dispromote(fromKoma);
			eachKomaBB[static_cast<size_t>(fromDispromKoma)].set(from);
			bammen[from] = static_cast<size_t>(fromDispromKoma);
		}
		else {//成らなかった場合
			fromKomaBB.set(from);
			bammen[from] = static_cast<size_t>(fromKoma);
		}
		if (captured != Koma::None) {//駒を取っていた場合
			eachKomaBB[static_cast<size_t>(captured)].set(to);
			bammen[KomaToMpos(captured)]--;
		}
	}
	else {//持ち駒から打っていた場合
		eachKomaBB[static_cast<size_t>(fromKoma)].reset(to);
		bammen[from]++;
	}
	bammen[to] = static_cast<uint8_t>(captured);
	return fromKoma;
}

std::uint64_t Kyokumen::getHash()const {
	auto& _p = allKomaBB.val();
	return _p[1] ^ (_p[0] << 22) ^ (_p[2] << 32)
		^ (static_cast<uint64_t>(bammen[81]) << 1) ^ (static_cast<uint64_t>(bammen[82]) << 6) 
		^ (static_cast<uint64_t>(bammen[83]) << 9) ^ (static_cast<uint64_t>(bammen[84]) << 12)
		^ (static_cast<uint64_t>(bammen[87]) << 15) ^ (static_cast<uint64_t>(bammen[85]) << 18) ^ (static_cast<uint64_t>(bammen[86]) << 20)
		^ (static_cast<uint64_t>(bammen[93]) << 43) ^ (static_cast<uint64_t>(bammen[92]) << 45) ^ (static_cast<uint64_t>(bammen[94]) << 47)
		^ (static_cast<uint64_t>(bammen[91]) << 50) ^ (static_cast<uint64_t>(bammen[90]) << 53)
		^ (static_cast<uint64_t>(bammen[89]) << 56) ^ (static_cast<uint64_t>(bammen[88]) << 59);
}

bool Kyokumen::isDeclarable()const {
	using namespace koma;
	//王手チェックはしないのでこの関数を呼ぶ前に確認しておくこと(パフォーマンス向上のため)
	if (teban()) {
		//宣言側の玉が敵陣三段目以内
		if (sOuPos() % 9 >= 3) {
			return false;
		}
		Bitboard tekijinSenteBB = senteKomaBB & bbmask::Dan1to3;
		//宣言側の敵陣三段目以内の駒が玉含めずに10枚以上存在する (玉含めて11枚以上)
		if (tekijinSenteBB.popcount() <= 10) {
			return false;
		}
		//宣言側の駒の点数が、大駒5点 小駒1点 玉0点で計算して、28点以上(後手は27点以上)
		int komapoint = 0;
		for (unsigned pos = tekijinSenteBB.pop_first(); pos < tekijinSenteBB.size(); pos = tekijinSenteBB.pop_first()) {
			const Koma koma = getKoma(pos);
			switch (koma) {
			case Koma::s_Fu:
			case Koma::s_Kyou:
			case Koma::s_Kei:
			case Koma::s_Gin:
			case Koma::s_Kin:
			case Koma::s_nFu:
			case Koma::s_nKyou:
			case Koma::s_nKei:
			case Koma::s_nGin:
				komapoint += 1;
				break;
			case Koma::s_Hi:
			case Koma::s_Kaku:
			case Koma::s_nHi:
			case Koma::s_nKaku:
				komapoint += 5;
				break;
			}
		}
		komapoint += getMochigomaNum(Position::m_sFu) * 1;
		komapoint += getMochigomaNum(Position::m_sKyou) * 1;
		komapoint += getMochigomaNum(Position::m_sKei) * 1;
		komapoint += getMochigomaNum(Position::m_sGin) * 1;
		komapoint += getMochigomaNum(Position::m_sKin) * 1;
		komapoint += getMochigomaNum(Position::m_sKaku) * 5;
		komapoint += getMochigomaNum(Position::m_sHi) * 5;
		if (komapoint >= 28)
			return true;
		else return false;
	}
	else {
		//宣言側の玉が敵陣三段目以内
		if (gOuPos() % 9 < 6) {
			return false;
		}
		Bitboard tekijinGoteBB = goteKomaBB & bbmask::Dan7to9;
		//宣言側の敵陣三段目以内の駒が玉含めずに10枚以上存在する (玉含めて11枚以上)
		if (tekijinGoteBB.popcount() <= 10) {
			return false;
		}
		//宣言側の駒の点数が、大駒5点 小駒1点 玉0点で計算して、27点以上(先手は28点以上)
		int komapoint = 0;
		for (unsigned pos = tekijinGoteBB.pop_first(); pos < tekijinGoteBB.size(); pos = tekijinGoteBB.pop_first()) {
			const Koma koma = getKoma(pos);
			switch (koma) {
			case Koma::g_Fu:
			case Koma::g_Kyou:
			case Koma::g_Kei:
			case Koma::g_Gin:
			case Koma::g_Kin:
			case Koma::g_nFu:
			case Koma::g_nKyou:
			case Koma::g_nKei:
			case Koma::g_nGin:
				komapoint += 1;
				break;
			case Koma::g_Hi:
			case Koma::g_Kaku:
			case Koma::g_nHi:
			case Koma::g_nKaku:
				komapoint += 5;
				break;
			}
		}
		komapoint += getMochigomaNum(Position::m_gFu) * 1;
		komapoint += getMochigomaNum(Position::m_gKyou) * 1;
		komapoint += getMochigomaNum(Position::m_gKei) * 1;
		komapoint += getMochigomaNum(Position::m_gGin) * 1;
		komapoint += getMochigomaNum(Position::m_gKin) * 1;
		komapoint += getMochigomaNum(Position::m_gKaku) * 5;
		komapoint += getMochigomaNum(Position::m_gHi) * 5;
		if (komapoint >= 27)
			return true;
		else return false;
	}
}

std::vector<Bitboard> Kyokumen::getSenteOuCheck(const Move m)const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = sOuPos();
	const unsigned from = m.from();
	const unsigned to = m.to();
	//親局面が存在しない、または玉自身が動いた場合は全体を調べる
	if (from == Position::NullMove || to==ouPos) {
		return getSenteOuCheck();
	}
	//fromでどいたところから空き王手がないか調べる
	if (isInside(from)) {
		Bitboard fpBB = pinMaskSente(from);
		if (fpBB != bbmask::AllOne) {
			fpBB.set(from);
			kusemono.push_back(fpBB);
		}
	}
	//toに移動した駒が玉に効いているか調べる
	const Koma movedKoma = getKoma(to);
	if (isDashable(movedKoma)) {
		Bitboard kiki = BBkiki::getDashKiki(allKomaBB, movedKoma, to);
		if ((kiki & getEachBB(Koma::s_Ou)).any()) {
			kiki.set(to);
			kiki &= BBkiki::getDashKiki(allKomaBB, sgInv(movedKoma), ouPos);
			kusemono.push_back(kiki);
		}
	}
	if(isSteppable(movedKoma)) {
		Bitboard tpBB = Bitboard(to);
		tpBB &= BBkiki::getStepKiki(sgInv(movedKoma), ouPos);
		if (tpBB.any()) {
			kusemono.push_back(tpBB);
		}
	}
	return kusemono;
}

std::vector<Bitboard> Kyokumen::getGoteOuCheck(const Move m)const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = gOuPos();
	const unsigned from = m.from();
	const unsigned to = m.to();
	//親局面が存在しない、または玉自身が動いた場合は全体を調べる
	if (from == Position::NullMove || to == ouPos) {
		return getGoteOuCheck();
	}
	//fromでどいたところから空き王手がないか調べる
	if (koma::isInside(from)) {
		Bitboard fpBB = pinMaskGote(from);
		if (fpBB != bbmask::AllOne) {
			fpBB.set(from);
			kusemono.push_back(fpBB);
		}
	}
	//toに移動した駒が玉に効いているか調べる
	const Koma movedKoma = getKoma(to);
	if (isDashable(movedKoma)) {
		Bitboard kiki = BBkiki::getDashKiki(allKomaBB, movedKoma, to);
		if ((kiki & getEachBB(Koma::g_Ou)).any()) {
			kiki.set(to);
			kiki &= BBkiki::getDashKiki(allKomaBB, sgInv(movedKoma), ouPos);
			kusemono.push_back(kiki);
		}
	}
	if(isSteppable(movedKoma)) {
		Bitboard tpBB = Bitboard(to);
		tpBB &= BBkiki::getStepKiki(sgInv(movedKoma), ouPos);
		if (tpBB.any()) {
			kusemono.push_back(tpBB);
		}
	}
	return kusemono;
}

std::vector<Bitboard> Kyokumen::getSenteOuCheck()const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = sOuPos();
	for (const Koma koma : gStepKomas) {
		Bitboard kusemonoBB = BBkiki::getStepKiki(sgInv(koma), ouPos) & getEachBB(koma);
		if (kusemonoBB.any()) {
			kusemono.push_back(kusemonoBB);
			break;
		}
	}
	for (const Koma koma : gDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(allKomaBB, sgInv(koma), ouPos);
		Bitboard eBB = kikiBB & getEachBB(koma);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard kusemonoBB = BBkiki::getDashKiki(allKomaBB, koma, i) & kikiBB;
				kusemonoBB.set(i);
				kusemono.push_back(kusemonoBB);
			}
		}
	}
	return kusemono;
}

std::vector<Bitboard> Kyokumen::getGoteOuCheck()const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = gOuPos();
	for (const Koma koma : sStepKomas) {
		Bitboard kusemonoBB = BBkiki::getStepKiki(sgInv(koma), ouPos) & getEachBB(koma);
		if (kusemonoBB.any()) {
			kusemono.push_back(kusemonoBB);
			break;
		}
	}
	for (const Koma koma : sDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(allKomaBB, sgInv(koma), ouPos);
		Bitboard eBB = kikiBB & getEachBB(koma);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard kusemonoBB = BBkiki::getDashKiki(allKomaBB, koma, i) & kikiBB;
				kusemonoBB.set(i);
				kusemono.push_back(kusemonoBB);
			}
		}
	}
	return kusemono;
}

bool Kyokumen::isOute(const Move& m)const {
	if (isSente) {
		return isSenteOute(m);
	}
	else {
		return isGoteOute(m);
	}
}

bool Kyokumen::isSenteOute(const Move m) const {
	const unsigned ouPos = sOuPos();
	const unsigned from = m.from();
	const unsigned to = m.to();
	//親局面が存在しない、または玉自身が動いた場合は全体を調べる
	if (from == Position::NullMove || to == ouPos) {
		return isSenteOute();
	}
	//fromでどいたところから空き王手がないか調べる
	if (isInside(from)) {
		Bitboard fpBB = pinMaskSente(from);
		if (fpBB != bbmask::AllOne) {
			return true;
		}
	}
	//toに移動した駒が玉に効いているか調べる
	const Koma movedKoma = getKoma(to);
	if (isDashable(movedKoma)) {
		Bitboard kiki = BBkiki::getDashKiki(allKomaBB, movedKoma, to);
		if ((kiki & getEachBB(Koma::s_Ou)).any()) {
			return true;
		}
	}
	if (isSteppable(movedKoma)) {
		Bitboard tpBB = Bitboard::genOneposBB(to);
		tpBB &= BBkiki::getStepKiki(sgInv(movedKoma), ouPos);
		if (tpBB.any()) {
			return true;
		}
	}
	return false;
}

bool Kyokumen::isGoteOute(const Move m) const {
	const unsigned ouPos = gOuPos();
	const unsigned from = m.from();
	const unsigned to = m.to();
	//親局面が存在しない、または玉自身が動いた場合は全体を調べる
	if (from == Position::NullMove || to == ouPos) {
		return isGoteOute();
	}
	//fromでどいたところから空き王手がないか調べる
	if (koma::isInside(from)) {
		Bitboard fpBB = pinMaskGote(from);
		if (fpBB != bbmask::AllOne) {
			return true;
		}
	}
	//toに移動した駒が玉に効いているか調べる
	const Koma movedKoma = getKoma(to);
	if (isDashable(movedKoma)) {
		Bitboard kiki = BBkiki::getDashKiki(allKomaBB, movedKoma, to);
		if ((kiki & getEachBB(Koma::g_Ou)).any()) {
			return true;
		}
	}
	if (isSteppable(movedKoma)) {
		Bitboard tpBB = Bitboard::genOneposBB(to);
		tpBB &= BBkiki::getStepKiki(sgInv(movedKoma), ouPos);
		if (tpBB.any()) {
			return true;
		}
	}
	return false;
}

bool Kyokumen::isSenteOute()const {
	const unsigned ouPos = sOuPos();
	for (const Koma koma : gStepKomas) {
		Bitboard kusemonoBB = BBkiki::getStepKiki(sgInv(koma), ouPos) & getEachBB(koma);
		if (kusemonoBB.any()) {
			return true;
		}
	}
	for (const Koma koma : gDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(allKomaBB, sgInv(koma), ouPos);
		Bitboard eBB = kikiBB & getEachBB(koma);
		if (eBB.any()) {
			return true;
		}
	}
	return false;
}

bool Kyokumen::isGoteOute()const {
	const unsigned ouPos = gOuPos();
	for (const Koma koma : sStepKomas) {
		Bitboard kusemonoBB = BBkiki::getStepKiki(sgInv(koma), ouPos) & getEachBB(koma);
		if (kusemonoBB.any()) {
			return true;
		}
	}
	for (const Koma koma : sDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(allKomaBB, sgInv(koma), ouPos);
		Bitboard eBB = kikiBB & getEachBB(koma);
		if (eBB.any()) {
			return true;
		}
	}
	return false;
}

Bitboard Kyokumen::pinMaskSente(const unsigned pos)const {
	const unsigned ouPos = sOuPos();
	Bitboard dpBB(allKomaBB);
	dpBB.reset(pos);
	for (Koma ek : gDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(dpBB, sgInv(ek), ouPos);
		Bitboard eBB = kikiBB & getEachBB(ek);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard result = kikiBB & BBkiki::getDashKiki(dpBB, ek, i);
				result.set(i);
				if (result.test(pos)) {
					return result;
				}
			}
		}
	}
	return bbmask::AllOne;
}

Bitboard Kyokumen::pinMaskGote(const unsigned pos)const {
	const unsigned ouPos = gOuPos();
	Bitboard dpBB(allKomaBB);
	dpBB.reset(pos);
	for (Koma ek : sDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(dpBB, sgInv(ek), ouPos);
		Bitboard eBB = kikiBB & getEachBB(ek);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard result = kikiBB & BBkiki::getDashKiki(dpBB, ek, i);
				result.set(i);
				if (result.test(pos)) {
					return result;
				}
			}
		}
	}
	return bbmask::AllOne;
}

Bitboard Kyokumen::senteKiki_ingnoreKing()const {
	Bitboard kikiBB;
	Bitboard enBB = senteKomaBB;
	enBB &= ~eachKomaBB[static_cast<size_t>(koma::Koma::s_Ou)];
	for (unsigned pos = enBB.pop_first(); pos != enBB.size(); pos = enBB.pop_first()) {
		const Koma k = getKoma(pos);
		kikiBB |= BBkiki::getKiki(allKomaBB, k, pos);
	}
	return kikiBB;
}

Bitboard Kyokumen::goteKiki_ingnoreKing()const {
	Bitboard kikiBB;
	Bitboard enBB = goteKomaBB;
	enBB &= ~eachKomaBB[static_cast<size_t>(koma::Koma::g_Ou)];
	for (unsigned pos = enBB.pop_first(); pos != enBB.size(); pos = enBB.pop_first()) {
		const Koma k = getKoma(pos);
		kikiBB |= BBkiki::getKiki(allKomaBB, k, pos);
	}
	return kikiBB;
}

bool Kyokumen::operator==(const Kyokumen& rhs) const {
	return (senteKomaBB == rhs.senteKomaBB && goteKomaBB == rhs.goteKomaBB && bammen == rhs.bammen);
}

void Kyokumen::reflectBitboard() {
	//bitboard
	for (auto& bb : eachKomaBB) {
		bb.all_reset();
	}
	for (int i = 0; i < 81; i++) {
		if (getKoma(i) != koma::Koma::None)
			eachKomaBB[bammen[i]].set(i);
	}
	senteKomaBB.all_reset();
	for (size_t i = static_cast<size_t>(koma::Koma::s_Min); i < static_cast<size_t>(koma::Koma::s_Num); i++) {
		senteKomaBB |= eachKomaBB[i]; //先手の駒をすべて集めたbb
	}
	goteKomaBB.all_reset();
	for (size_t i = static_cast<size_t>(koma::Koma::g_Min); i < static_cast<size_t>(koma::Koma::g_Num); i++) {
		goteKomaBB |= eachKomaBB[i]; //後手の駒をすべて集めたbb
	}
	allKomaBB = senteKomaBB | goteKomaBB;//全体のbbは先後のものを合成する
}

std::string Kyokumen::toBanFigure()const {
	std::string fig;
	fig += "teban: "; 
	fig += (teban() ? "sente\n" : "gote\n");
	for (int y = 0; y < 9; y++) {
		for (int x = 9 - 1; x >= 0; x--) {
			Koma k = getKoma(static_cast<Position>(x * 9 + y));
			auto s = (k != koma::Koma::None) ? usi::komaToUsi(k) : "-";
			if (s.length() == 1) {
				s = ' ' + s;
			}
			fig += s + ' ';
		}
		fig += '\n';
	}
	fig += "SenteMochi: ";
	std::string smochistr;
	for (Mochigoma m = Mochigoma::Fu; m != Mochigoma::MochigomaNum; m = static_cast<Mochigoma>(static_cast<uint8_t>(m) + 1)) {
		int mNum = getMochigomaNum(true, m);
		if (mNum > 0) {
			smochistr += usi::mochigomaToUsi(true, m) + std::to_string(mNum) + " ";
		}
	}
	if (smochistr == "") smochistr = "none";
	fig += smochistr;
	fig += '\n';
	fig += "GoteMochi: ";
	std::string gmochistr;
	for (Mochigoma m = Mochigoma::Fu; m != Mochigoma::MochigomaNum; m = static_cast<Mochigoma>(static_cast<uint8_t>(m) + 1)) {
		int mNum = getMochigomaNum(false, m);
		if (mNum > 0) {
			gmochistr += usi::mochigomaToUsi(false, m) + std::to_string(mNum) + " ";
		}
	}
	if (gmochistr == "") gmochistr = "none";
	fig += gmochistr;
	return fig;
}