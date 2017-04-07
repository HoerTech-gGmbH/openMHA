function delays = measure_delay(jack_output, jack_input)
% measures sound delay using 3 different methods:
%
% (1) jack_delay by fons adriaensen
% (2) jackgetirs from the MHA distribution
% (3) getirs from the Tascar Toolbox
%
% Returns a vector containing these 3 delays in seconds.

  delays(1) = measure_delay_jack_delay(jack_output, jack_input)
  delays(2) = measure_delay_jackgetirs(jack_output, jack_input)
end

function jack_delay = measure_delay_jack_delay(jack_output, jack_input)
  [status,jack_delay] = system(sprintf('jack_delay -I "%s" -O "%s"', jack_input, jack_output));
  if status ~= 0
    error('jack_delay returned error status %d',status)
  end
  jack_delay = textscan(jack_delay);
  jack_delay = jack_delay{1}(3) / 1000;
end

function jack_delay = measure_delay_jackgetirs(jack_output, jack_input)
  command = sprintf('jackgetirs delay_correction=no "capture=[%s]" "playback=[%s]"', ...
		    jack_input, jack_output);
  [status] = system(command);
  if status ~= 0
    error('jack_getirs returned error status %d',status)
  end
  [irs,fs,nbits] = wavread('jackgetirs.wav');
  [dummy,index] = max(abs(irs));
  jack_delay = (index-1) / fs;
end
