function [output_signal] = mha_process_by_parser(mha, input_signal)
%
% Process a long signal by dividing it into chunks and sending them to MHAIOParser
%
% output_signal:  wave output of mha (ch,samples)
% input_signal:   wave input for mha (ch,samples)
% handle:     an MHA-handle, derived by mha = mha_start();

  nchannels_in = mha_get(mha,'nchannels_in');
  fragsize = mha_get(mha,'fragsize');
  output_signal = {};
  for startindex = [1:fragsize:size(input_signal,2)]
    endindex = min([startindex + fragsize - 1, size(input_signal,2)]);
    chunk = zeros(nchannels_in, fragsize);
    chunk(:,1:(endindex-startindex+1)) = input_signal(:,startindex:endindex);
    %try
      mha_set (mha,'io.input',chunk);
      answer = mha_get(mha,'io.output');
    %catch
    %  err = lasterror;
    %  mha_set(mha,'cmd','quit');
   %   rethrow(err);
   % end
    output_signal = [output_signal, {answer}];
  end
  output_signal = cell2mat(output_signal);
  %size(output_signal)
  output_signal = output_signal(:,1:size(input_signal,2));
  %size(output_signal)
  %mha_set(mha,'cmd','quit');
