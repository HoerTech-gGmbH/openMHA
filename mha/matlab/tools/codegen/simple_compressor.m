function output_signal = simple_compressor(input_signal)

global gaintable;
global attack_coeffs;
global release_coeffs;
global gaintable_levels;

persistent state;

nchannels_in = size(input_signal,2);
%fragsize = size(input_signal,1);

if isempty(state)
    state = ones(1,nchannels_in)*65;
end

assert(size(gaintable,1) == nchannels_in);
assert(size(attack_coeffs,2) == nchannels_in);
assert(size(attack_coeffs,1) == 1);
assert(isequal(size(release_coeffs), size(attack_coeffs)));
assert(size(gaintable_levels,1) == 1);
assert(size(gaintable_levels,2) == size(gaintable,2));

meansquares = mean(input_signal.^2);
instant_levels = 10*log10(meansquares / 4e-10 + 1e-10);
filtered_levels = state;

output_signal = input_signal;
for channel = 1:nchannels_in
    coeff = release_coeffs(channel);
    if (instant_levels(channel) > filtered_levels(channel))
        coeff = attack_coeffs(channel);
    end
    filtered_levels(channel) = filtered_levels(channel) * coeff + (1-coeff) * instant_levels(channel);
    gain = interp1(gaintable_levels, gaintable(channel,:), filtered_levels(channel));
    output_signal(:,channel) = input_signal(:,channel) * 10.^(gain/20);
end

state = filtered_levels;
