function [output_signal] = mha_process_vector(host, port, input_signal, fragsize)
% output_signal = mha_process_signal(host, port, input_signal)
%
% output_signal:  wave output of mha
% input_signal:   wave input for mha
% host:     the host name of the mha server
% port:     the port number at which the MHAIOTCP module listens for
%           network sound streams.

  nchannels_in = size(input_signal, 2);
  handle = mha_tcpsound(host, port);
  output_signal = {};
  for startindex = [1:fragsize:size(input_signal,1)]
    endindex = min([startindex + fragsize - 1, size(input_signal,1)]);
    chunk = zeros(fragsize, nchannels_in);
    chunk(1:(endindex-startindex+1),:) = input_signal(startindex:endindex,:);
    try
      answer = mha_tcpsound(handle, chunk);
    catch
      err = lasterror;
      mha_tcpsound(handle, 'close');
      rethrow(err);
    end
    output_signal = [output_signal; {answer}];
  end
  output_signal = cell2mat(output_signal);
  output_signal = output_signal(1:size(input_signal,1),:);
  mha_tcpsound(handle, 'close');
