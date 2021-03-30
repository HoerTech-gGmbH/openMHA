function [s_out,user_config,state] = process(s_in,signal_dimensions,user_config,state)
%PROCESS
% Multiply input spectrum element wise with filter coefficients
s_out=user_config(1).value'.*s_in;
end
