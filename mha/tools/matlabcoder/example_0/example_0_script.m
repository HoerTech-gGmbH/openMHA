% EXAMPLE_0_SCRIPT   Generate static library process from process.
% 
% Script generated from project 'example_0.prj' on 23-Sep-2020.
% 
% See also CODER, CODER.CONFIG, CODER.TYPEOF, CODEGEN.

%% Create configuration object of class 'coder.CodeConfig'.
cfg = coder.config('lib','ecoder',false);
cfg.GenerateReport = true;
cfg.ReportPotentialDifferences = false;
cfg.DynamicMemoryAllocation = 'Off';
cfg.SaturateOnIntegerOverflow = false;
cfg.FilePartitionMethod = 'SingleFile';
cfg.GenCodeOnly = true;
cfg.SupportNonFinite = false;
cfg.HardwareImplementation.ProdHWDeviceType = 'Generic->Unspecified (assume 32-bit Generic)';
cfg.HardwareImplementation.TargetHWDeviceType = 'Generic->Unspecified (assume 32-bit Generic)';

%% Define argument types for entry-point 'process'.
ARGS = cell(1,1);
ARGS{1} = cell(1,1);
ARGS{1}{1} = coder.typeof(single(0),[2 64]);

%% Invoke MATLAB Coder.
codegen -config cfg process -args ARGS{1}
