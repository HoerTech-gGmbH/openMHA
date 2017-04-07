function [output_signal] = mha_process(mha_handle, input_signal)
% output_signal = mha_process_signal(mha_handle, input_signal)
%
% output_signal:  wave output of mha
% input_signal:   wave input for mha
% mha_handle:     the handle returned by mha_setup
%
% In the place of mha_handle, you can also call mha_process with the same
% mha_desciption struct you would call setup with.


  allocate_mha = ~isfield(mha_handle, ...
			  'this_field_was_installed_by_mha_setup');
  if allocate_mha
    dsc = mha_handle;
    mha_handle = mha_start();
    mha_set(mha_handle,'',dsc)
  end
  
  mhactl_wrapper(mha_handle.tcp.cmd, ...
         {'?saveshort:mha.logcfg', '?savemons:mha.logmon'});
  
  if size(input_signal, 2) ~= mha_handle.nchannels_in
    if allocate_mha
      mha_teardown(mha_handle);
    end
    error('wrong number of channels in input signal');
  end

  output_signal = {};
  for startindex = [1:mha_handle.wndshift:size(input_signal,1)]
    endindex = min([startindex + mha_handle.wndshift - 1, size(input_signal,1)]);
    chunk = zeros(mha_handle.wndshift, size(input_signal,2));
    chunk(1:(endindex-startindex+1),:) = input_signal(startindex:endindex,:);
    try
      answer = mha_tcpsound(mha_handle.tcp.sound.handle, chunk);
    catch
      err = lasterror;
      if allocate_mha
	mha_teardown(mha_handle);
      end
      rethrow(err);
    end
    output_signal = [output_signal; {answer}];
  end
  output_signal = cell2mat(output_signal);
  output_signal = output_signal(1:size(input_signal,1),:);

  if allocate_mha
    mha_teardown(mha_handle);
  end
