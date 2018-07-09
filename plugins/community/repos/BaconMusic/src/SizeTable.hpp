#ifndef INCLUDE_SIZE_TABLE
#define INCLUDE_SIZE_TABLE


// Wish there was a better way to do this
template< typename T >
struct SizeTable
{
};

template<>
struct SizeTable< PJ301MPort >
{
  constexpr static const float X = 24.6721;
  constexpr static const float Y = 24.6721;
};

template<>
struct SizeTable< RoundHugeBlackKnob >
{
  constexpr static const float X = 56.1034;
  constexpr static const float Y = 56.1034;
};

template<>
struct SizeTable< RoundLargeBlackKnob >
{
  constexpr static const float X = 37.5;
  constexpr static const float Y = 37.5;
};

template<>
struct SizeTable< RoundBlackSnapKnob >
{
  constexpr static const float X = 29.5287;
  constexpr static const float Y = 29.5287;
};

template<>
struct SizeTable< RoundBlackKnob >
{
  constexpr static const float X = 29.5287;
  constexpr static const float Y = 29.5287;
};

template<>
struct SizeTable< DotMatrixLightTextWidget >
{
  constexpr static const float X = 11;
  constexpr static const float Y = 18.5;
};

template<>
struct SizeTable< RoundSmallBlackKnob >
{
  constexpr static const float X = 23.6206;
  constexpr static const float Y = 23.6206;
};

// Diff Y to center
template< typename T1, typename T2 >
float diffY2c()
{
  return ( SizeTable<T1>::Y - SizeTable<T2>::Y ) / 2;
}

#endif
