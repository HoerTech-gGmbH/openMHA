function [signal_dimensions, user_config]=prepare(signal_dimensions, user_config)
%PREPARE 
% Optional, prepare global data, check dimensions, etc..
if(signal_dimensions.domain~='W')
    fprintf('This plugin can only process signals in the time domain. Got %s\n',signal_dimensions.domain);assert(false);
end

% Need one delay entry per input channel
user_config(1).value=zeros(signal_dimensions.channels,1);

% Need one gain entry per input channel
user_config(2).value=ones(signal_dimensions.channels,1);

% Number of output channels is always one
signal_dimensions.channels=uint32(1);
end
