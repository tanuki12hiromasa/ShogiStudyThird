#include "../stdafx.h"
#include "nnue_evaluate.h"
#include <fstream>
#include <thread>
#include <filesystem>

namespace Eval::NNUE {


	namespace Detail {
		template <typename T>
		bool ReadParameters(std::istream& stream, const T* pointer) {
			std::uint32_t header;
			stream.read(reinterpret_cast<char*>(&header), sizeof(header));
			if (!stream || header != T::GetHashValue()) {
				return false;
			}
			return pointer->ReadParameters(stream);
		}
		template <typename T>
		bool ReadParameters(std::istream& stream) {
			std::uint32_t header;
			stream.read(reinterpret_cast<char*>(&header), sizeof(header));
			if (!stream || header != T::GetHashValue()) {
				return false;
			}
			return T::ReadParameters(stream);
		}
		template <typename T>
		bool WriteParameters(std::ostream& stream, const T* pointer) {
			constexpr std::uint32_t header = T::GetHashValue();
			stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
			return pointer->WriteParameters(stream);
		}
		template <typename T>
		bool WriteParameters(std::ostream& stream) {
			constexpr std::uint32_t header = T::GetHashValue();
			stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
			return T::WriteParameters(stream);
		}
	}

	std::unique_ptr<Network> NNUE_evaluator::network;
	std::string NNUE_evaluator::ifolderpath = "./nnue";
	std::string NNUE_evaluator::ofolderpath = "./learn/nnue";

	bool NNUE_evaluator::ReadHeader(std::istream& stream, std::uint32_t* hash_value, std::string* architecture) {
		std::uint32_t version, size;
		stream.read(reinterpret_cast<char*>(&version), sizeof(version));
		stream.read(reinterpret_cast<char*>(hash_value), sizeof(*hash_value));
		stream.read(reinterpret_cast<char*>(&size), sizeof(size));
		if (!stream || version != kVersion)
			return false;
		architecture->resize(size);
		stream.read(&(*architecture)[0], size);
		return !stream.fail();
	}

	bool NNUE_evaluator::WriteHeader(std::ostream& stream, std::uint32_t* hash_value, std::string& architecture) {
		stream.write(reinterpret_cast<const char*>(&kVersion), sizeof(kVersion));
		stream.write(reinterpret_cast<const char*>(&hash_value), sizeof(hash_value));
		const std::uint32_t size = static_cast<std::uint32_t>(architecture.size());
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
		stream.write(architecture.data(), size);
		return !stream.fail();
	}

	bool NNUE_evaluator::ReadParameters(std::istream& stream) {
		std::uint32_t hash_value;
		std::string architecture;
		if (!ReadHeader(stream, &hash_value, &architecture)
			|| hash_value != kHashValue
			|| !Detail::ReadParameters<NNUE_feat>(stream)
			|| !Detail::ReadParameters(stream,network.get())) {
			return false;
		}
		return stream && stream.peek() == std::ios::traits_type::eof();
	}

	bool NNUE_evaluator::WriteParameters(std::ostream& stream) {

	}

	void NNUE_evaluator::init() {
		//メモリの確保
		if (network == nullptr) {
			network = std::unique_ptr<Network>(new Network);
		}
		NNUE_feat::init();
		
		//パラメータの読み込み
		std::ifstream stream(ifolderpath + "/nn.bin", std::ios::binary);
		const bool result = ReadParameters(stream);
		if (!result) {
			using namespace std::chrono_literals;
			std::cout << "error:failed to read " << ifolderpath << "/nn.bin" << std::endl;
			std::this_thread::sleep_for(500ms);
			::exit(EXIT_FAILURE);
		}
		std::cout << "Parameters have been read from " << ifolderpath << std::endl;
	}

	void NNUE_evaluator::save() {
		std::filesystem::create_directories(ofolderpath);

	}

	double NNUE_evaluator::evaluate(const SearchPlayer& player) {
		alignas(kCacheLineSize) TransformedFeatureType transformed_features[NNUE_feat::kBufferSize];
		player.feature.Transform(player.kyokumen, transformed_features);
		alignas(kCacheLineSize) char buffer[Network::kBufferSize];
		const auto output = network->Propagate(transformed_features, buffer);
		double score = static_cast<double>(output[0]) / FV_SCALE;
		return score;
	}
}