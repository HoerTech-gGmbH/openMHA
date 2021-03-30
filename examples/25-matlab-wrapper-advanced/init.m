function [user_config,state] = init(user_config,state)
%INIT
% Initialize 'delay' and 'gain' user configuration variables
user_config =[struct('name','delay', 'value',ones(1,1)); struct('name','gain', 'value',ones(1,1))];
end
