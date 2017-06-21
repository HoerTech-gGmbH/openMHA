function p = thispath
% returns the name of the directory that contains the mfile of the caller.

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: thispath.m,v 1.3 2005/04/13 16:17:09 tobiasl Exp $

bt = backtrace;
p = fileparts(bt(2).file);
