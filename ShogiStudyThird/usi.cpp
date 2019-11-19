#include "stdafx.h"
#include "usi.h"
#include <sstream>

using namespace koma;
namespace usi {
	static const std::array<std::string, static_cast<size_t>(Koma::KomaNum)> KomaUsi = {
		"P","L","N","S","B","R","G","K",
		"+P","+L","+N","+S","+B","+R",
		"p","l","n","s","b","r","g","k",
		"+p","+l","+n","+s","+b","+r"
	};
	std::string komaToUsi(Koma koma) { return KomaUsi[static_cast<size_t>(koma)]; }//Koma::Noneは渡してはいけないので注意
	std::string mochigomaToUsi(bool teban, Mochigoma koma) {
		if (teban)
			return KomaUsi[static_cast<size_t>(koma)];
		else
			return KomaUsi[static_cast<size_t>(koma) + 14];
	}
	std::string mposToUsi(Position mpos) {
		assert(mpos >= Position::SQm_Min && mpos < Position::SQm_Num);
		if (mpos < Position::m_gFu) {
			return KomaUsi[mpos - Position::m_sFu];
		}
		else {
			return KomaUsi[(size_t)(mpos - Position::m_gFu) + 14u];
		}
	}

	Mochigoma usiToMochi(const char usi) {
		switch (usi) {
		case 'P': return Mochigoma::Fu;
		case 'L': return Mochigoma::Kyou;
		case 'N': return Mochigoma::Kei;
		case 'S': return Mochigoma::Gin;
		case 'B': return Mochigoma::Kaku;
		case 'R': return Mochigoma::Hi;
		case 'G': return Mochigoma::Kin;
		case 'p': return Mochigoma::Fu;
		case 'l': return Mochigoma::Kyou;
		case 'n': return Mochigoma::Kei;
		case 's': return Mochigoma::Gin;
		case 'b': return Mochigoma::Kaku;
		case 'r': return Mochigoma::Hi;
		case 'g': return Mochigoma::Kin;
		default: assert(0); return Mochigoma::None;
		}
	}
	Koma usiToKoma(const char usi) {
		switch (usi) {
		case 'P': return Koma::s_Fu;
		case 'L': return Koma::s_Kyou;
		case 'N': return Koma::s_Kei;
		case 'S': return Koma::s_Gin;
		case 'B': return Koma::s_Kaku;
		case 'R': return Koma::s_Hi;
		case 'G': return Koma::s_Kin;
		case 'K': return Koma::s_Ou;
		case 'p': return Koma::g_Fu;
		case 'l': return Koma::g_Kyou;
		case 'n': return Koma::g_Kei;
		case 's': return Koma::g_Gin;
		case 'b': return Koma::g_Kaku;
		case 'r': return Koma::g_Hi;
		case 'g': return Koma::g_Kin;
		case 'k': return Koma::g_Ou;
		default: return Koma::None;
		}
	}

	const std::array<char, 9> ColumUsi = { '1','2','3','4','5','6','7','8','9' };
	const std::array<char, 9> RawUsi = { 'a','b','c','d','e','f','g','h','i' };

	std::string posToUsi(const Position pos) {
		std::string str;
		str += ColumUsi[PosToSuji(pos)];
		str += RawUsi[PosToDan(pos)];
		return str;
	}

	char tebanToUsi(bool teban) { return teban ? 'b' : 'w'; }

	std::vector<std::string> split(const std::string& str, char splitter) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;
		while (std::getline(ss, token, splitter)) {
			tokens.push_back(token);
		}
		return tokens;
	}

	std::string btos(bool b) { return b ? "true" : "false"; }
}
