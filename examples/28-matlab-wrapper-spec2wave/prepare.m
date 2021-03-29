function [signal_dimensions, user_config]=prepare(signal_dimensions, user_config)
%PREPARE
% Checks for correct number of channels(1) and input domain (S). Sets
% output domain to waveform.
if(signal_dimensions.domain~='S')
    fprintf('This plugin can only process signals in the spectral domain. Got %s.\n',signal_dimensions.domain);assert(false);
end
if(signal_dimensions.channels~=1)
    fprintf('This plugin can only process mono signals. Got %u channels.\n',signal_dimensions.channels);assert(false);
end
signal_dimensions.domain='W';
signal_dimensions.channels=uint32(1);
end
