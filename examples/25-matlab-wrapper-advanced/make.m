% This script generates code for the functions init, prepare, process, and release.
% The input and output argument types are chosen according to the interface of the
% matlab_wrapper plugin.
% Please read the matlab coder integration manual for more information

%% Create configuration object of class 'coder.CodeConfig'.
cfg = coder.config('dll','ecoder',false);
cfg.GenerateReport = true;
cfg.ReportPotentialDifferences = false;
cfg.SaturateOnIntegerOverflow = false;
cfg.FilePartitionMethod = 'SingleFile';
cfg.EnableOpenMP = false;
cfg.RuntimeChecks = true;
cfg.SupportNonFinite = false;
cfg.HardwareImplementation.ProdHWDeviceType = 'Generic->32-bit x86 compatible';
cfg.HardwareImplementation.ProdLongLongMode = true;
cfg.HardwareImplementation.TargetLongLongMode = true;

%% Define argument types for entry-point 'init'.
ARGS = cell(4,1);
ARGS{1} = cell(1,1);
ARGS_1_1 = struct;
ARGS_1_1.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_1_1.value = coder.typeof(0,[Inf Inf],[1 1]);
ARGS{1}{1} = coder.typeof(ARGS_1_1,[Inf  1],[1 0]);
ARGS{1}{1} = coder.cstructname(ARGS{1}{1},'user_config_t');

%% Define argument types for entry-point 'prepare'.
ARGS{2} = cell(2,1);
ARGS_2_1 = struct;
ARGS_2_1.channels = coder.typeof(uint32(0));
ARGS_2_1.domain = coder.typeof('X');
ARGS_2_1.fragsize = coder.typeof(uint32(0));
ARGS_2_1.wndlen = coder.typeof(uint32(0));
ARGS_2_1.fftlen = coder.typeof(uint32(0));
ARGS_2_1.srate = coder.typeof(0);
ARGS{2}{1} = coder.typeof(ARGS_2_1);
ARGS{2}{1} = coder.cstructname(ARGS{2}{1},'signal_dimensions_t');
ARGS_2_2 = struct;
ARGS_2_2.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_2_2.value = coder.typeof(0,[Inf Inf],[1 1]);
ARGS{2}{2} = coder.typeof(ARGS_2_2,[Inf  1],[1 0]);
ARGS{2}{2} = coder.cstructname(ARGS{2}{2},'user_config_t');

%% Define argument types for entry-point 'process'.
ARGS{3} = cell(3,1);
ARGS{3}{1} = coder.typeof(0,[Inf Inf],[1 1]);
ARGS_3_2 = struct;
ARGS_3_2.channels = coder.typeof(uint32(0));
ARGS_3_2.domain = coder.typeof('X');
ARGS_3_2.fragsize = coder.typeof(uint32(0));
ARGS_3_2.wndlen = coder.typeof(uint32(0));
ARGS_3_2.fftlen = coder.typeof(uint32(0));
ARGS_3_2.srate = coder.typeof(0);
ARGS{3}{2} = coder.typeof(ARGS_3_2);
ARGS{3}{2} = coder.cstructname(ARGS{3}{2},'signal_dimensions_t');
ARGS_3_3 = struct;
ARGS_3_3.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_3_3.value = coder.typeof(0,[Inf  Inf],[1 0]);
ARGS{3}{3} = coder.typeof(ARGS_3_3,[Inf  1],[1 0]);
ARGS{3}{3} = coder.cstructname(ARGS{3}{3},'user_config_t');

%% Invoke MATLAB Coder.
codegen -config cfg -o example_25 init -args ARGS{1} prepare -args ARGS{2} process -args ARGS{3} release

%% Optionally package the code for deployment elsewhere
%load('codegen\dll\example_25\buildinfo.mat')
%packNGo(buildInfo,'fileName','example_25.zip');
