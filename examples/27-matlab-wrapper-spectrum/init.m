function [user_config,state] = init(user_config,state)
%INIT 
% Initialize user_config vector with 'filter' config variable
user_config =[struct('name','filter', 'value',ones(1,1))];
end
