function success = wait_for_jack(timeout)
  if ispc()
    disp([char(10),'----- WAIT FOR JACK -----',char(10)]);
    pause(timeout); % Jack 1.9.11 on Windows has no jack_wait.exe
    success = true;
  else
    success = false;
    [status, output] = system(sprintf('jack_wait -w -t %d', timeout));
    if (status == 0)
      if isequal(output, sprintf('server is available\n'))
        success = true;
      end
    end
  end
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
