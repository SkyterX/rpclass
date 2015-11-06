#pragma once

#include <bitset>

namespace arcflags
{
	namespace bitset
	{
		template <size_t size>
		class Bitset
		{
		public:
			Bitset(){}

			bool GetBit(size_t position) const
			{
				return bitset[position];
			}

			void SetBit(size_t position, bool value = true)
			{
				bitset.set(position, value);
			}

		private:
			std::bitset<size> bitset;
		};
	}
}

