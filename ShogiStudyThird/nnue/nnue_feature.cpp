#include "../stdafx.h"
#include "nnue_feature.h"

namespace Eval::NNUE {

#define VECTOR
	//ベクトル命令
#ifdef USE_AVX2
using vec_t = __m256i;
static constexpr IndexType kNumRegs = 16;
#define vec_load(a) _mm256_load_si256(a)
#define vec_store(a, b) _mm256_store_si256(a, b)
#define vec_add_16(a, b) _mm256_add_epi16(a, b)
#define vec_sub_16(a, b) _mm256_sub_epi16(a, b)
#define vec_zero _mm256_setzero_si256()
#endif

	void EvalList::set(const Kyokumen& kyokumen) {
		using namespace ::koma;
		int nlist = 0;
#define Mochi(index0,index1,koma,teban)\
 for(int32_t i=kyokumen.getMochigomaNum(teban, koma); i>0; --i) {\
	 assert(nlist < 38);\
	 list[0][nlist]=index0+i; list[1][nlist]=index1+i;\
	 ++nlist; \
 }
		Mochi(f_hand_pawn, e_hand_pawn, Mochigoma::Fu, true);
		Mochi(e_hand_pawn, f_hand_pawn, Mochigoma::Fu, false);
		Mochi(f_hand_lance, e_hand_lance, Mochigoma::Kyou, true);
		Mochi(e_hand_lance, f_hand_lance, Mochigoma::Kyou, false);
		Mochi(f_hand_knight, e_hand_knight, Mochigoma::Kei, true);
		Mochi(e_hand_knight, f_hand_knight, Mochigoma::Kei, false);
		Mochi(f_hand_silver, e_hand_silver, Mochigoma::Gin, true);
		Mochi(e_hand_silver, f_hand_silver, Mochigoma::Gin, false);
		Mochi(f_hand_gold, e_hand_gold, Mochigoma::Kin, true);
		Mochi(e_hand_gold, f_hand_gold, Mochigoma::Kin, false);
		Mochi(f_hand_bishop, e_hand_bishop, Mochigoma::Kaku, true);
		Mochi(e_hand_bishop, f_hand_bishop, Mochigoma::Kaku, false);
		Mochi(f_hand_rook, e_hand_rook, Mochigoma::Hi, true);
		Mochi(e_hand_rook, f_hand_rook, Mochigoma::Hi, false);
#undef Mochi
		Bitboard bb = kyokumen.getAllBB();
		bb &= ~(kyokumen.getEachBB(Koma::s_Ou) | kyokumen.getEachBB(Koma::g_Ou));
		for (int i = bb.find_first(); i < bb.size(); i = bb.find_next(i)) {
			assert(nlist < 38);
			Koma koma = kyokumen.getKoma(i);
			list[0][nlist] = komaToIndex(koma) + i;
			list[1][nlist] = komaToIndex(sgInv(koma)) + inverse(i);
			++nlist;
		}
		assert(nlist == 38);
	}
	
	void EvalList::proceed(const Kyokumen& before, const Move& move, ChangedIndices& removed, ChangedIndices& added, std::array<int, 2>& kPos, std::array<bool, 2>& reset) {
		for (auto& r : reset) { r = false; }
		for (auto& a : added) { a[0] = fe_end; a[1] = fe_end; }
		for (auto& rm : removed) { rm[0] = fe_end; rm[1] = fe_end; }

		//before:置き換え元のEvalIndex, after:新しいEvalIndex, pers:先後の視点(perspective) 0or1, num:変更した駒が何個目か 0or1
#define Replace(before,after,pers,num) {int i;\
		for(i=0;i<EvalList::EvalListSize;i++){\
			if(list[pers][i]==before){\
				removed[pers][num]=before;\
				list[pers][i]=after;\
				added[pers][num]=after;\
				break;}\
		}assert(i<38);}

		using namespace ::koma;
		const auto from = move.from();
		const auto to = move.to();
		if (!isInside(from)) {
			kPos[0] = before.sOuPos();
			kPos[1] = inverse(before.gOuPos());
			const Mochigoma m = MposToMochi(from);
			const Koma k = MposToKoma(from);
			const int mNum = before.getMochigomaNum(before.teban(), m);
			const EvalIndex bIndex0 = mochiToIndex(m, before.teban()) + mNum;//b:before, a:after, 0:先手視点, 1:後手視点
			const EvalIndex bIndex1 = mochiToIndex(m, !before.teban()) + mNum;
			const EvalIndex aIndex0 = komaToIndex(k) + to;
			const EvalIndex aIndex1 = komaToIndex(sgInv(k)) + inverse(to);
			Replace(bIndex0, aIndex0, 0, 0);
			Replace(bIndex1, aIndex1, 1, 0);
		}
		else {
			const Koma k = before.getKoma(from);
			const Koma cap = before.getKoma(to);
			if (k == Koma::s_Ou) {
				kPos[0] = to;
				reset[0] = true;
				kPos[1] = inverse(before.gOuPos());
				if (cap != Koma::None) {
					const Mochigoma m_cap = KomaToMochi(cap);
					const int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					const EvalIndex bcIndex0 = komaToIndex(cap) + to;
					const EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(to);
					const EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					const EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					Replace(bcIndex0, acIndex0, 0, 0);
					Replace(bcIndex1, acIndex1, 1, 0);
				}
			}
			else if (k == Koma::g_Ou) {
				kPos[0] = before.sOuPos();
				kPos[1] = inverse(to);
				reset[1] = true;
				if (cap != Koma::None) {
					const Mochigoma m_cap = KomaToMochi(cap);
					const int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					const EvalIndex bcIndex0 = komaToIndex(cap) + to;
					const EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(to);
					const EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					const EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					Replace(bcIndex0, acIndex0, 0, 0);
					Replace(bcIndex1, acIndex1, 1, 0);
				}
			}
			else {
				kPos[0] = before.sOuPos();
				kPos[1] = inverse(before.gOuPos());
				const EvalIndex bIndex0 = komaToIndex(k) + from;
				const EvalIndex bIndex1 = komaToIndex(sgInv(k)) + inverse(from);
				const Koma ak = move.promote() ? promote(k) : k;
				const EvalIndex aIndex0 = komaToIndex(ak) + to;
				const EvalIndex aIndex1 = komaToIndex(sgInv(ak)) + inverse(to);
				Replace(bIndex0, aIndex0, 0, 0);
				Replace(bIndex1, aIndex1, 1, 0);
				if (cap != Koma::None) {
					const Mochigoma m_cap = KomaToMochi(cap);
					const int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					const EvalIndex bcIndex0 = komaToIndex(cap) + to;
					const EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(to);
					const EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					const EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					Replace(bcIndex0, acIndex0, 0, 1);
					Replace(bcIndex1, acIndex1, 1, 1);
				}
			}
		}

#undef Replace
	}

	void EvalList::recede(const Kyokumen& before, const koma::Koma& moved, const koma::Koma& captured, const Move& move, ChangedIndices& removed, ChangedIndices& added, std::array<int, 2>& kPos, std::array<bool, 2>& reset) {
		for (auto& r : reset) { r = false; }
		for (auto& a : added) { a[0] = fe_end; a[1] = fe_end; }
		for (auto& rm : removed) { rm[0] = fe_end; rm[1] = fe_end; }

#define Replace(before,after,pers,num) {int i;\
		for(i=0;i<EvalList::EvalListSize;i++){\
			if(list[pers][i]==before){\
				removed[pers][num]=before;\
				list[pers][i]=after;\
				added[pers][num]=after;\
				break;}\
		}assert(i<38);}
		
		using namespace ::koma;
		const auto from = move.from();
		const auto to = move.to();

		if (!isInside(from)) {
			kPos[0] = before.sOuPos();
			kPos[1] = inverse(before.gOuPos());
			const Mochigoma m = MposToMochi(from);
			const Koma k = MposToKoma(from);
			const int mNum = before.getMochigomaNum(before.teban(), m);
			const EvalIndex bIndex0 = mochiToIndex(m, before.teban()) + mNum;//b:before, a:after, 0:先手視点, 1:後手視点
			const EvalIndex bIndex1 = mochiToIndex(m, !before.teban()) + mNum;
			const EvalIndex aIndex0 = komaToIndex(k) + to;
			const EvalIndex aIndex1 = komaToIndex(sgInv(k)) + inverse(to);
			Replace(aIndex0, bIndex0, 0, 0);
			Replace(aIndex1, bIndex1, 1, 0);
		}
		else {
			const Koma k = before.getKoma(from);
			const Koma cap = before.getKoma(to);
			if (k == Koma::s_Ou) {
				kPos[0] = before.sOuPos();
				reset[0] = true;
				kPos[1] = inverse(before.gOuPos());
				if (cap != Koma::None) {
					const Mochigoma m_cap = KomaToMochi(cap);
					const int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					const EvalIndex bcIndex0 = komaToIndex(cap) + to;
					const EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(to);
					const EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					const EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					Replace(acIndex0, bcIndex0, 0, 0);
					Replace(acIndex1, bcIndex1, 1, 0);
				}
			}
			else if (k == Koma::g_Ou) {
				kPos[0] = before.sOuPos();
				kPos[1] = inverse(before.gOuPos());
				reset[1] = true;
				if (cap != Koma::None) {
					const Mochigoma m_cap = KomaToMochi(cap);
					const int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					const EvalIndex bcIndex0 = komaToIndex(cap) + to;
					const EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(to);
					const EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					const EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					Replace(acIndex0, bcIndex0, 0, 0);
					Replace(acIndex1, bcIndex1, 1, 0);
				}
			}
			else {
				kPos[0] = before.sOuPos();
				kPos[1] = inverse(before.gOuPos());
				const EvalIndex bIndex0 = komaToIndex(k) + from;
				const EvalIndex bIndex1 = komaToIndex(sgInv(k)) + inverse(from);
				const Koma ak = move.promote() ? promote(k) : k;
				const EvalIndex aIndex0 = komaToIndex(ak) + to;
				const EvalIndex aIndex1 = komaToIndex(sgInv(ak)) + inverse(to);
				Replace(aIndex0, bIndex0, 0, 0);
				Replace(aIndex1, bIndex1, 1, 0);
				if (cap != Koma::None) {
					const Mochigoma m_cap = KomaToMochi(cap);
					const int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					const EvalIndex bcIndex0 = komaToIndex(cap) + to;
					const EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(to);
					const EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					const EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					Replace(acIndex0, bcIndex0, 0, 1);
					Replace(acIndex1, bcIndex1, 1, 1);
				}
			}
		}

#undef Replace
	}

	NNUE_feat::BiasType* NNUE_feat::biases_;
	NNUE_feat::WeightType* NNUE_feat::weights_;

	void NNUE_feat::init() {
		if(biases_ == nullptr) biases_ = new BiasType[kTransformedFeatureDimensions];
		if(weights_ == nullptr) weights_ = new WeightType[kTransformedFeatureDimensions * kInputDimensions];
	}

	std::string NNUE_feat::GetStructureString() {
		return "HalfKP(friend)[" + std::to_string(kInputDimensions) + "->" + std::to_string(kHalfDimensions) + "x2]";
	}

	bool NNUE_feat::ReadParameters(std::istream& stream) {
		for (std::size_t i = 0; i < kHalfDimensions; ++i) biases_[i] = read_little_endian<BiasType>(stream);
		for (std::size_t i = 0; i < kHalfDimensions * kInputDimensions; ++i)
			weights_[i] = read_little_endian<WeightType>(stream);
		return !stream.fail();
	}

	bool NNUE_feat::WriteParameters(std::ostream& stream) {
		stream.write(reinterpret_cast<const char*>(biases_), kHalfDimensions * sizeof(BiasType));
		stream.write(reinterpret_cast<const char*>(weights_), kHalfDimensions * kInputDimensions * sizeof(WeightType));
		return !stream.fail();
	}

	void NNUE_feat::set(const Kyokumen& kyokumen) {
		indexlist.set(kyokumen);
		refreshAccumulator(kyokumen);
	}

	void NNUE_feat::refreshAccumulator(const Kyokumen& kyokumen) {
		for (int perspective : {0, 1}) {
			std::memcpy(accumulator.accumulation[perspective], biases_, kHalfDimensions * sizeof(BiasType));
			const int kpos = (perspective == 0) ? kyokumen.sOuPos() : kyokumen.gOuPos();
			for (const auto& p_index : indexlist.list[perspective]) {
				const IndexType index = kpos * fe_end + p_index;
				const IndexType offset = kHalfDimensions * index;
#if defined(VECTOR)
				auto accumulation = reinterpret_cast<vec_t*>(accumulator.accumulation[perspective]);
				auto column = reinterpret_cast<const vec_t*>(weights_ + offset);
				constexpr IndexType kNumChunks = kHalfDimensions / (kSimdWidth / 2);
				for (IndexType j = 0; j < kNumChunks; ++j) {
					accumulation[j] = vec_add_16(accumulation[j], column[j]);
				}
#else
				for (IndexType j = 0; j < kHalfDimensions; ++j) {
					accumulator.accumulation[perspective][j] += weights_[offset + j];
				}
#endif
			}
		}
	}

	void NNUE_feat::updateAccumulator(const std::array<int, 2>& kpos, const ChangedIndices& removed, const ChangedIndices& added, const std::array<bool, 2>& reset) {
		for (int perspective : {0, 1}) {
#if defined(VECTOR)
			constexpr IndexType kNumChunks = kHalfDimensions / (kSimdWidth / 2);
			auto accumulation = reinterpret_cast<vec_t*>(&accumulator.accumulation[perspective][0]);
#endif
			if (reset[perspective]) {
				std::memcpy(accumulator.accumulation[perspective], biases_, kHalfDimensions * sizeof(BiasType));
				for (const auto& p_index : indexlist.list[perspective]) {
					const IndexType index = kpos[perspective] * fe_end + p_index;
					const IndexType offset = kHalfDimensions * index;
#if defined(VECTOR)
					auto accumulation = reinterpret_cast<vec_t*>(accumulator.accumulation[perspective]);
					auto column = reinterpret_cast<const vec_t*>(weights_ + offset);
					constexpr IndexType kNumChunks = kHalfDimensions / (kSimdWidth / 2);
					for (IndexType j = 0; j < kNumChunks; ++j) {
						accumulation[j] = vec_add_16(accumulation[j], column[j]);
					}
#else
					for (IndexType j = 0; j < kHalfDimensions; ++j) {
						accumulator.accumulation[perspective][j] += weights_[offset + j];
					}
#endif
				}
			}
			else {
				// Difference calculation for the feature amount changed from 1 to 0
				// 1から0に変化した特徴量に関する差分計算
				for (const auto p_index : removed[perspective]) {
					if (p_index == fe_end) continue;
					const IndexType index = kpos[perspective] * fe_end + p_index;
					const IndexType offset = kHalfDimensions * index;
#if defined(VECTOR)
					auto column = reinterpret_cast<const vec_t*>(&weights_[offset]);
					for (IndexType j = 0; j < kNumChunks; ++j) {
						accumulation[j] = vec_sub_16(accumulation[j], column[j]);
					}
#else
					for (IndexType j = 0; j < kHalfDimensions; ++j) {
						accumulator.accumulation[perspective][i][j] -= weights_[offset + j];
					}
#endif
				}
				// Difference calculation for features that changed from 0 to 1
				// 0から1に変化した特徴量に関する差分計算
				for (const auto p_index : added[perspective]) {
					const IndexType index = kpos[perspective] * fe_end + p_index;
					const IndexType offset = kHalfDimensions * index;
#if defined(VECTOR)
					auto column = reinterpret_cast<const vec_t*>(&weights_[offset]);
					for (IndexType j = 0; j < kNumChunks; ++j) {
						accumulation[j] = vec_add_16(accumulation[j], column[j]);
					}
#else
					for (IndexType j = 0; j < kHalfDimensions; ++j) {
						accumulator.accumulation[perspective][i][j] += weights_[offset + j];
					}
#endif
				}
			}
		}
	}

	void NNUE_feat::Transform(const Kyokumen& kyokumen, OutputType output[]) const {
		/*if (refresh) {
			refreshAccumulator(kyokumen);
		}*/
		const auto& accumulation = accumulator.accumulation;
#if defined(USE_AVX512)
		constexpr IndexType kNumChunks = kHalfDimensions / (kSimdWidth * 2);
		static_assert(kHalfDimensions % (kSimdWidth * 2) == 0);
		const __m512i kControl = _mm512_setr_epi64(0, 2, 4, 6, 1, 3, 5, 7);
		const __m512i kZero = _mm512_setzero_si512();
#elif defined(USE_AVX2)
		constexpr IndexType kNumChunks = kHalfDimensions / kSimdWidth;
		constexpr int       kControl = 0b11011000;
		const __m256i       kZero = _mm256_setzero_si256();
#elif defined(USE_SSE2)
		constexpr IndexType kNumChunks = kHalfDimensions / kSimdWidth;
#if defined(USE_SSE41)
		const __m128i kZero = _mm_setzero_si128();
#else  // SSE41非対応だがSSE2は使える環境
		const __m128i k0x80s = _mm_set1_epi8(-128);
#endif
#endif
		const int perspectives[2] = { tebanToInt(kyokumen.teban()),tebanToInt(!kyokumen.teban()) };
		for (IndexType p = 0; p < 2; p++) {
			const IndexType offset = kHalfDimensions * p;
#if defined(USE_AVX512)
			auto out = reinterpret_cast<__m512i*>(&output[offset]);
			for (IndexType j = 0; j < kNumChunks; ++j) {
				__m512i sum0 =
					_mm512_load_si512(&reinterpret_cast<const __m512i*>(accumulation[perspectives[p]])[j * 2 + 0]);
				__m512i sum1 =
					_mm512_load_si512(&reinterpret_cast<const __m512i*>(accumulation[perspectives[p]])[j * 2 + 1]);
				_mm512_store_si512(&out[j], _mm512_permutexvar_epi64(
					kControl, _mm512_max_epi8(_mm512_packs_epi16(sum0, sum1), kZero)));
			}

#elif defined(USE_AVX2)
			auto out = reinterpret_cast<__m256i*>(&output[offset]);
			for (IndexType j = 0; j < kNumChunks; ++j) {
				__m256i sum0 =
					_mm256_load_si256(&reinterpret_cast<const __m256i*>(accumulation[perspectives[p]])[j * 2 + 0]);
				__m256i sum1 =
					_mm256_load_si256(&reinterpret_cast<const __m256i*>(accumulation[perspectives[p]])[j * 2 + 1]);

				_mm256_store_si256(&out[j], _mm256_permute4x64_epi64(
					_mm256_max_epi8(_mm256_packs_epi16(sum0, sum1), kZero), kControl));
			}

#elif defined(USE_SSE2)
			auto out = reinterpret_cast<__m128i*>(&output[offset]);
			for (IndexType j = 0; j < kNumChunks; ++j) {
				__m128i sum0 =
					_mm_load_si128(&reinterpret_cast<const __m128i*>(accumulation[perspectives[p]])[j * 2 + 0]);
				__m128i sum1 =
					_mm_load_si128(&reinterpret_cast<const __m128i*>(accumulation[perspectives[p]])[j * 2 + 1]);

				const __m128i packedbytes = _mm_packs_epi16(sum0, sum1);
				_mm_store_si128(&out[j],
#if defined(USE_SSE41)
					_mm_max_epi8(packedbytes, kZero)
#else  // SSE41非対応だがSSE2は使える環境
					_mm_subs_epi8(_mm_adds_epi8(packedbytes, k0x80s), k0x80s)
#endif
				);
			}
#endif

		}

	}

	void NNUE_feat::proceed(const Kyokumen& before, const Move& move) {
		ChangedIndices added, removed;
		std::array<int, 2> kpos;
		std::array<bool, 2> reset;
		indexlist.proceed(before, move, removed, added, kpos, reset);
		updateAccumulator(kpos, removed, added, reset);
	}

	void NNUE_feat::recede(const Kyokumen& before, const koma::Koma moved, const koma::Koma captured, const Move move, const Cache& cache) {
		ChangedIndices added, removed;
		std::array<int, 2> kpos;
		std::array<bool, 2> reset;
		indexlist.recede(before, moved, captured, move, removed, added, kpos, reset);
		updateAccumulator(kpos, removed, added, reset);
	}

	std::string NNUE_feat::toString() const {
		std::string str;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < kHalfDimensions; j++) {
				str += std::to_string(accumulator.accumulation[i][j]);
				str += " ";
			}
			str += "\n";
		}
		str += "list0: ";
		for (const auto val : indexlist.list[0])str += std::to_string(val) + " ";
		str += "\nlist1: ";
		for (const auto val : indexlist.list[1])str += std::to_string(val) + " ";
		str += "\n";
		return str;
	}

	bool NNUE_feat::operator==(const NNUE_feat& rhs)const {
#ifdef _DEBUG
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < kTransformedFeatureDimensions; j++) {
				if (accumulator.accumulation[i][j] != rhs.accumulator.accumulation[i][j]) {
					return false;
				}
			}
		}
#endif
		return indexlist.list == rhs.indexlist.list;
	}
}