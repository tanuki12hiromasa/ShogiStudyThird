#pragma once

#include "layers/input_slice.h"
#include "layers/affine_transform.h"
#include "layers/clipped_relu.h"
#include "layers/sum.h"
#include "nnue_param.h"

#include "../kyokumen.h"

namespace Eval {
	namespace NNUE {

		constexpr IndexType kTransformedFeatureDimensions = 256;
		namespace Layers {
			using InputLayer = InputSlice<kTransformedFeatureDimensions * 2>;
			using HiddenLayer1 = ClippedReLU<AffineTransform<InputLayer, 32>>;
			using HiddenLayer2 = ClippedReLU<AffineTransform<HiddenLayer1, 32>>;
			using OutputLayer = AffineTransform<HiddenLayer2, 1>;

		}
		using Network = Layers::OutputLayer;

		struct alignas(32) Accumulator {
			std::int16_t accumulation[2][kTransformedFeatureDimensions];//元コードでは[2][1][256]
			Accumulator():accumulation(){}
		};

		using ChangedIndices = std::array<std::array<EvalIndex, 2>, 2>;

		struct EvalList {
			static constexpr int EvalListSize = 38;
			std::array<std::array<EvalIndex, EvalListSize>, 2> list;

			EvalList() :list{ {f_hand_pawn} } {} //search_treeはコンストラクトを持たないのでデフォルトコンストラクタが要る
			EvalList(const Kyokumen& k) { set(k); }
			void set(const Kyokumen&);
			void proceed(const Kyokumen& before, const Move& move, ChangedIndices& removed, ChangedIndices& added, std::array<int, 2>& kPos, std::array<bool, 2>& reset);
			void recede(const Kyokumen& before, const koma::Koma& moved, const koma::Koma& captured, const Move& move, ChangedIndices& removed, ChangedIndices& added, std::array<int, 2>& kPos, std::array<bool, 2>& reset);
		};

		class Cache{};

		class NNUE_feat {
		public:
			using BiasType = std::int16_t;
			using WeightType = std::int16_t;
			using OutputType = TransformedFeatureType;
			
			static constexpr IndexType kInputDimensions = EvalIndex::fe_end * SquareNum;//125388U;
			static constexpr IndexType kOutputDimentions = kTransformedFeatureDimensions * 2;
			static constexpr IndexType kHalfDimensions = kTransformedFeatureDimensions;
			static constexpr std::size_t kBufferSize = kOutputDimentions * sizeof(OutputType);
		private:
			static BiasType* biases_; //BiasType biases_[kTransformedFeatureDimensions];
			static WeightType* weights_; //WeightType weights_[kTransformedFeatureDimensions * kInputDimensions];
		public:
			static constexpr std::uint32_t GetHashValue() { return 0x5D69D5B9u ^ 1 ^ kOutputDimentions; }
			static std::string GetStructureString();
			static bool ReadParameters(std::istream& stream);
			static bool WriteParameters(std::ostream& stream);
			static void init();

		public:
			NNUE_feat(){}
			NNUE_feat(const Kyokumen& kyokumen) :indexlist(kyokumen) { set(kyokumen); }
			void set(const Kyokumen& kyokumen);
			void refreshAccumulator(const Kyokumen& kyokumen);
			void updateAccumulator(const std::array<int,2>& kpos, const ChangedIndices& removed, const ChangedIndices& added, const std::array<bool, 2>& reset);
			void Transform(const Kyokumen& kyokumen, OutputType Output[])const;
			void proceed(const Kyokumen& before, const Move& move);
			void recede(const Kyokumen& before, const koma::Koma moved, const koma::Koma captured, const Move move, const Cache& cache);
			Cache getCache() { return Cache(); }
			std::string toString()const;

			bool operator==(const NNUE_feat& rhs)const;
			bool operator!=(const NNUE_feat& rhs)const { return !operator==(rhs); }

		private:
			Accumulator accumulator;
			EvalList indexlist;
		};

	}
}
