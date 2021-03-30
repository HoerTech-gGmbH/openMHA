function [signal_dimensions, user_config,state]=prepare(signal_dimensions, user_config,state)
%PREPARE 
% Optional, prepare global data, check dimensions, etc..
if(signal_dimensions.domain~='W')
    fprintf('This plugin can only process signals in the time domain. Got %s\n',signal_dimensions.domain);assert(false);
end
% We only want to process stereo signals
if(signal_dimensions.channels~=2)
    fprintf('Number of channels must be 2. Got %u.\n',signal_dimensions.channels);assert(false);
end
end
