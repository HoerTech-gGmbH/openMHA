function disp_err_rethrow
% DISP_ERR_RETHROW - display error message and backtrace
%
% Author: Giso Grimm
  err = lasterror;
  errordlg(err.message);
  disp(err.message);
  for k=1:length(err.stack)
    disp(sprintf('%s:%d %s',...
		 err.stack(k).file,...
		 err.stack(k).line,...
		 err.stack(k).name));
  end
  rethrow(err);
