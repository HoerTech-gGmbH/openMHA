function [s_out,user_config,state] = process_ws(s_in,signal_dimensions,user_config,state)
% This function provides the entry point for waveform to spectrum
% processing and forwards the actual processing to process(...)
% Do not touch unless you know what you are doing! 
% The actual processing should be done in process.m
[s_out,user_config,state]=process(s_in,signal_dimensions,user_config,state);
end
