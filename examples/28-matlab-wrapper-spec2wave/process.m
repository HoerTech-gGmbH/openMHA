function [s_out,user_config] = process(s_in,signal_dimensions,user_config)
%PROCESS
% A simple vocoder implementation. For every frequency in the frequency
% vector adds a sinusoid proportional to the magitude of the corresponding 
% frequency bin in the input spectrum

% Shorthand for the frequency vector
frequencies=user_config(1).value;

% Initialize output signal
s_out=zeros(signal_dimensions.fragsize,1);
for f=1:size(frequencies,2)
    % Get level corresponding to frequency
    level=abs(s_in(fft_find_bin(frequencies(f),double(signal_dimensions.fftlen),double(signal_dimensions.srate))));
    % Calculate phase advance per sample. Slightly fudge the frequency so
    % that the sinus fits exactly in the audio block to reduce
    % discontinuities.
    whole_periods_per_block=double(round(double(signal_dimensions.fragsize)*frequencies(f)/double(signal_dimensions.srate)));
    phase_advance_per_sample=double(whole_periods_per_block)*2*pi/double(signal_dimensions.fragsize);
    phase=0.0;
    % Add the sine
    for i=1:signal_dimensions.fragsize
        s_out(i)=s_out(i)+level*sin(phase);
        phase=phase+phase_advance_per_sample;
    end
end
end

% Helper function to find the fft bin of a given frequency
function bin=fft_find_bin(frequency,fftlen, srate)
    bin=round(frequency*fftlen/srate)+1;
end
