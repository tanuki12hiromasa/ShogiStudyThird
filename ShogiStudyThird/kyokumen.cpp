#include "stdafx.h"
#include "kyokumen.h"
#include "usi.h"

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

Kyokumen::Kyokumen():
	Kyokumen(prime_bammen,true){}

Kyokumen::Kyokumen(const std::array<koma::Koma, 81>& ban, const std::array<unsigned, 7>& smochi, const std::array<unsigned, 7>& gmochi,bool teban)
	: bammen({0}), isSente(teban)
{
	std::copy(ban.begin(), ban.end(), bammen.begin());
	std::copy(smochi.begin(), smochi.end(), bammen.begin() + 81);
	std::copy(gmochi.begin(), gmochi.end(), bammen.begin() + 88);
	reflectBitboard();
}

Kyokumen::Kyokumen(const std::array < std::uint8_t, 95 >& bammen, bool teban)
	:bammen(bammen), isSente(teban)
{
	reflectBitboard();
}

Kyokumen::Kyokumen(std::array < std::uint8_t, 95 > && bammen, bool teban)
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
			if (bammen[x * 9 + y] != static_cast<std::uint8_t>(Koma::None)) {
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

koma::Position Kyokumen::proceed(const Move move) {
	const unsigned from = move.from(), to = move.to();
	const bool prom = move.promote();
	Position captured = Position::SQm_Num;
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
		const Koma toKoma = getKoma(to);
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
		if (toKoma != Koma::None) {//駒を取っていた場合 
			Bitboard& toKomaBB = eachKomaBB[static_cast<size_t>(toKoma)];
			toKomaBB.reset(to);//BB cap
			Position capMpos = captured = KomaToMpos(toKoma);
			bammen[capMpos]++;//mban cap
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