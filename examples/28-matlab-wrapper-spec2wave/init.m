function [user_config,state] = init(user_config,state)
%INIT 
% Initialize user configurable frequency vector
user_config =[struct('name','frequencies', 'value',ones(1,1))];
state =[struct('name','real_frequencies', 'value',ones(1,20))];
end
