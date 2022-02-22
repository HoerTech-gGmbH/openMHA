// Most target architectures are little-endian with IEC559/IEEE754 floats
// This header provides conversion functions for all others and
// functions to write binary data to streambufs

#ifndef CONVERSIONS_H_
#define CONVERSIONS_H_

#include <cstdint>
#include <limits>
#include <ostream>
#include <vector>

#ifdef EXOTIC_ARCH_SUPPORT
#include <boost/version.hpp>

// support for endianness and binary floating-point storage
// this import scheme is part of the portable_archive code by
// christian.pfligersdorffer@eos.info (under boost license)
#include <boost/spirit/home/support/detail/math/fpclassify.hpp>
// namespace alias fp_classify
namespace fp = boost::spirit::math;

#if BOOST_VERSION >= 105800
#include <boost/endian/conversion.hpp>
using boost::endian::native_to_little_inplace;
#else
#error "BOOST_VERSION >= 1.58 required"
#endif

// === writer functions ===
// write an integer value in little endian
// derived from portable archive code by christian.pfligersdorffer@eos.info (under boost license)
template <typename T>
typename std::enable_if<std::is_integral<T>::value>::type write_little_endian(
	std::ostream &dst, T t) {
	native_to_little_inplace(t);
	dst->sputn(reinterpret_cast<const char *>(&t), sizeof(t));
}

// write a floating-point value in little endian
// derived from portable archive code by christian.pfligersdorffer@eos.info (under boost license)
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value>::type write_little_endian(
	std::ostream &dst, T t) {
	// Get a type big enough to hold
	using traits = typename fp::detail::fp_traits<T>::type;
	static_assert(sizeof(typename traits::bits) == sizeof(T),
		"floating point type can't be represented accurately");

	typename traits::bits bits;
	// remap to bit representation
	switch (fp::fpclassify(t)) {
	case FP_NAN: bits = traits::exponent | traits::mantissa; break;
	case FP_INFINITE: bits = traits::exponent | (t < 0) * traits::sign; break;
	case FP_SUBNORMAL: break;
	case FP_ZERO: // note that floats can be ±0.0
	case FP_NORMAL: traits::get_bits(t, bits); break;
	default: bits = 0; break;
	}
	write_little_endian(dst, bits);
}

// store a sample's values to a stream (numeric version) */
template <class T>
inline const T *write_sample_values(std::ostream &dst, const T *sample, std::size_t len) {
	// [Value1] .. [ValueN] */
	for (const T *end = sample + len; sample < end; ++sample) write_little_endian(dst, *sample);
	return sample;
}

#else

static_assert(std::numeric_limits<float>::is_iec559,
	"Non-IEC559/IEEE754-floats need EXOTIC_ARCH_SUPPORT defined");
static_assert(std::numeric_limits<float>::has_denorm, "denormalized floats not supported");
static_assert(sizeof(float) == 4, "Unexpected float size!");
static_assert(sizeof(double) == 8, "Unexpected double size!");

template <typename T>
typename std::enable_if<sizeof(T) == 1>::type write_little_endian(std::ostream &dst, T t) {
	dst.put(t);
}

template <typename T>
typename std::enable_if<sizeof(T) >= 2>::type write_little_endian(std::ostream &dst, T t) {
	dst.write(reinterpret_cast<const char *>(&t), sizeof(T));
}

template <class T>
inline const T *write_sample_values(std::ostream &dst, const T *sample, std::size_t len) {
	// [Value1] .. [ValueN] */
	dst.write(reinterpret_cast<const char *>(sample), len * sizeof(T));
	return sample + len;
}
#endif

template <class T>
inline void write_sample_values(std::ostream &dst, const std::vector<T> &sample) {
	write_sample_values(dst, sample.data(), sample.size());
}

template <typename T>
inline void write_sample_values(std::ostream &buf, const std::vector<std::vector<T>> &vecs) {
	for (const std::vector<T> &vec : vecs) write_sample_values(buf, vec);
}

// write a variable-length integer (int8, int32, or int64)
inline void write_varlen_int(std::ostream &dst, uint64_t val) {
	if (val < 256) {
		dst.put(1);
		dst.put(static_cast<uint8_t>(val));
	} else if (val <= 4294967295) {
		dst.put(4);
		write_little_endian(dst, static_cast<uint32_t>(val));
	} else {
		dst.put(8);
		write_little_endian(dst, static_cast<uint64_t>(val));
	}
}

template <typename T> inline void write_fixlen_int(std::ostream &dst, T val) {
	dst.put(sizeof(T));
	write_little_endian(dst, val);
}


// store a sample's values to a stream (string version)
template <>
inline const std::string *write_sample_values(
	std::ostream &dst, const std::string *sample, std::size_t len) {
	// [Value1] .. [ValueN] */
	for (const std::string *end = sample + len; sample < end; ++sample) {
		// [NumLengthBytes], [Length] (as varlen int)
		write_varlen_int(dst, sample->size());
		// [StringContent] */
		dst.write(sample->data(), sample->size());
	}
	return sample;
}

#endif
