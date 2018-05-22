function ola_delay = compute_ola_delay(wndlen, wndshift, fftlen, wndpos)
if nargin < 4
    wndpos = 0.5;
endif
ola_delay = floor(wndlen-wndshift + (fftlen - wndlen) * wndpos);
endfunction