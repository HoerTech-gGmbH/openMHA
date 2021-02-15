function switched_signal = channel_switch(signal_frag)
    % Function takes a 2 channel signal fragment and switches the channels
    A = [0 1; 1 0];
    switched_signal = A * signal_frag;
end
