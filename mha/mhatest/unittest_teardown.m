function unittest_teardown(varargin)
% unittest_teardown(varargin)
% 
% This function registers and executes teardowns.
%
% Call unittest_teardown like you would call feval. The registered
% function (with arguments) will be executed by runtests.m after your
% test case finishes.
%
% Multiple teardowns can be registered, and will be executed in reverse
% order (LIFO).
  
% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: unittest_teardown.m,v 1.3 2005/04/13 16:17:09 tobiasl Exp $

  persistent stored_teardown;
  if nargin == 0
    % execute stored teardowns
    for index = 1:length(stored_teardown)
      feval(stored_teardown{index}{:});
    end
    % each teardown is executed at most once
    stored_teardown = {};
  else
    if isequal([], varargin{1})
      % make sure no teardown is registered
      stored_teardown = {};
    else
      % register teardown
      if isempty(stored_teardown)
        stored_teardown = [{varargin}];
      else
        stored_teardown = [{varargin} stored_teardown];
      end
    end
  end
