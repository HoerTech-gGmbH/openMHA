function [PSM, PSMt, ODG, PSM_inst] = audiomeasure_measure_pemoq(RefSig, TestSig, fs,varargin )
  [PSM, PSMt, ODG, PSM_inst] = ...
      audioqual(RefSig, TestSig, fs, varargin{:} );