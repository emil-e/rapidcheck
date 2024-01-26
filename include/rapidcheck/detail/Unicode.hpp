#pragma once

namespace rc {
namespace detail {

template<typename T, typename RandomType>
T generateCodePoint(rc::detail::BitStream<RandomType>& stream)
{
	static_assert(sizeof T >= 3, 
		"Code points can only be stored in types at leeast three bytes large.");

	// Note, this algorithm is designed to provide
	// good values for UTF8 encoding but can be
	// used to generate any Unicode character
	int maxBytes = 1;

	T codepoint;
	while (maxBytes < 4)
	{
		bool increase = stream.next<bool>();
		if (!increase)
		{
			break;
		}
		maxBytes += 1;
	}
	int noBits;
	switch (maxBytes)
	{
	case 1:
		noBits = 7;
		break;
	case 2:
		noBits = 11;
		break;
	case 3:
		noBits = 16;
		break;
	default:
		noBits = 20;
		// Actually 21, put the first bit
		// needs to be specially handled
		// to not exceed the valid
		// value range for codepoints
		bool highestBit = stream.next<bool>();
		if (highestBit)
		{
			return 0x100000 | stream.next<T>(16);
		}

	}

	do
	{
		codepoint = stream.next<T>(noBits);
	} while (codepoint == 0);
	return codepoint;
}

template<typename T, typename Y>
T makeCharacterUtf8(Y codepoint)
{
	using ValType = T::value_type;
	if (codepoint <= 0x7F)
	{
		return{ static_cast<ValType>(codepoint) };
	}
	else if (codepoint <= 0x7FF)
	{
		return{
			static_cast<ValType>(0b11000000 | ((codepoint >> (6)) & 0b00011111)),
			static_cast<ValType>(0b10000000 | ((codepoint) & 0b00111111))
		};
	}
	else if (codepoint <= 0xFFFF)
	{
		return{
			static_cast<ValType>(0b11100000 | ((codepoint >> (6 + 6)) & 0b00001111)),
			static_cast<ValType>(0b10000000 | ((codepoint >> (6)) & 0b00111111)),
			static_cast<ValType>(0b10000000 | ((codepoint) & 0b00111111))
		};
	}
	else if (codepoint <= 0x10FFFF)
	{
		return{
			static_cast<ValType>(0b11110000 | ((codepoint >> (6+6+6)) & 0b00000111)),
			static_cast<ValType>(0b10000000 | ((codepoint >> (6+6)) & 0b00111111)),
			static_cast<ValType>(0b10000000 | ((codepoint >> (6)) & 0b00111111)),
			static_cast<ValType>(0b10000000 | ((codepoint) & 0b00111111))
		};
	}
	return T();
}

} // namespace detail
} // namespace rc