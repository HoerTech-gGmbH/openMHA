function [user_config,state] = init(user_config,state)
%INIT
% Initialize 'gain' user configuration variable
user_config =[struct('name','gain', 'value',ones(1,1))];
end
