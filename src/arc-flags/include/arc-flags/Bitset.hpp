#pragma once

#include <bitset>

namespace arcflags {
	namespace bitset {
		template <size_t size>
		class Bitset {
			using BitsetType = std::bitset<size>;
		public:
			Bitset() {}

			bool GetBit(size_t position) const {
				return bitset[position];
			}

			void SetBit(size_t position, bool value = true) {
				bitset.set(position, value);
			}

			size_t GetHashCode() const {
				return hasher(bitset);
			}

			friend bool operator==(const Bitset& lhs, const Bitset& rhs) {
				return lhs.bitset == rhs.bitset;
			}

			friend bool operator!=(const Bitset& lhs, const Bitset& rhs) {
				return !(lhs == rhs);
			}

		private:
			const static std::hash<BitsetType> hasher;
			BitsetType bitset;
		};
	}
}

namespace std {
	template <size_t size>
	struct hash<arcflags::bitset::Bitset<size>>
		: public unary_function<arcflags::bitset::Bitset<size>, size_t> {
		size_t operator()(const arcflags::bitset::Bitset<size>& bitset) const {
			return bitset.GetHashCode();
		}
	};
}
