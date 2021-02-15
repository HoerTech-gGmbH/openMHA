fragsize = 64;
signal = audioread('test.wav')';
%Fragment based processing
for i=1:fragsize:length(signal)
    signal_fragment = single(signal(:, i:i+fragsize-1));
    processed_signal(:, i:i+fragsize-1) = process(signal_fragment);
end

%Test if the functions works as intended
assert(isequal(signal(1, :), processed_signal(2,:)))
assert(isequal(signal(2, :), processed_signal(1,:)))
