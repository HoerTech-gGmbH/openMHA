function set_simple_compressor_parameters(channels, sampling_rate, block_size, gaintable_levels_, gaintable_, attack_times, release_times)

global gaintable;
global attack_coeffs;
global release_coeffs;
global gaintable_levels;

gaintable = gaintable_;
gaintable_levels = gaintable_levels_;

assert(sampling_rate > 0);
assert(block_size > 0);
assert(size(gaintable,1) == channels);
assert(size(attack_times,2) == channels);
assert(size(attack_times,1) == 1);
assert(isequal(size(release_times), size(attack_times)));
assert(size(gaintable_levels,1) == 1);
assert(size(gaintable_levels,2) == size(gaintable,2));

block_rate = sampling_rate / double(block_size);
attack_coeffs = exp(-1./(attack_times * block_rate));
release_coeffs = exp(-1./(release_times * block_rate));
