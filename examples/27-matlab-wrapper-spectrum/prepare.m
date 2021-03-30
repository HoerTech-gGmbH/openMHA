function [signal_dimensions, user_config,state]=prepare(signal_dimensions, user_config,state)
%PREPARE
% Checks for correct input domain.
if(signal_dimensions.domain~='S')
    fprintf('This plugin can only process signals in the spectral domain. Got %s\n',signal_dimensions.domain);assert(false);
end
end
