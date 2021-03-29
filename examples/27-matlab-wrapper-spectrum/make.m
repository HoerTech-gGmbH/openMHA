% See also CODER, CODER.CONFIG, CODER.TYPEOF, CODEGEN.

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

%% Define argument types for entry-point 'process_ww'.
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

%% Define argument types for entry-point 'process_ss'.
ARGS{4} = cell(3,1);
ARGS{4}{1} = coder.typeof(complex(0),[Inf Inf],[1 1]);
ARGS_4_2 = struct;
ARGS_4_2.channels = coder.typeof(uint32(0));
ARGS_4_2.domain = coder.typeof('X');
ARGS_4_2.fragsize = coder.typeof(uint32(0));
ARGS_4_2.wndlen = coder.typeof(uint32(0));
ARGS_4_2.fftlen = coder.typeof(uint32(0));
ARGS_4_2.srate = coder.typeof(0);
ARGS{4}{2} = coder.typeof(ARGS_4_2);
ARGS{4}{2} = coder.cstructname(ARGS{4}{2},'signal_dimensions_t');
ARGS_4_3 = struct;
ARGS_4_3.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_4_3.value = coder.typeof(0,[Inf  Inf],[1 0]);
ARGS{4}{3} = coder.typeof(ARGS_4_3,[Inf  1],[1 0]);
ARGS{4}{3} = coder.cstructname(ARGS{4}{3},'user_config_t');

%% Define argument types for entry-point 'process_ws'.
ARGS{5} = cell(3,1);
ARGS{5}{1} = coder.typeof(0,[Inf Inf],[1 1]);
ARGS_5_2 = struct;
ARGS_5_2.channels = coder.typeof(uint32(0));
ARGS_5_2.domain = coder.typeof('X');
ARGS_5_2.fragsize = coder.typeof(uint32(0));
ARGS_5_2.wndlen = coder.typeof(uint32(0));
ARGS_5_2.fftlen = coder.typeof(uint32(0));
ARGS_5_2.srate = coder.typeof(0);
ARGS{5}{2} = coder.typeof(ARGS_4_2);
ARGS{5}{2} = coder.cstructname(ARGS{5}{2},'signal_dimensions_t');
ARGS_5_3 = struct;
ARGS_5_3.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_5_3.value = coder.typeof(0,[Inf  Inf],[1 0]);
ARGS{5}{3} = coder.typeof(ARGS_5_3,[Inf  1],[1 0]);
ARGS{5}{3} = coder.cstructname(ARGS{5}{3},'user_config_t');

%% Define argument types for entry-point 'process_sw'.
ARGS{6} = cell(3,1);
ARGS{6}{1} = coder.typeof(complex(0),[Inf Inf],[1 1]);
ARGS_6_2 = struct;
ARGS_6_2.channels = coder.typeof(uint32(0));
ARGS_6_2.domain = coder.typeof('X');
ARGS_6_2.fragsize = coder.typeof(uint32(0));
ARGS_6_2.wndlen = coder.typeof(uint32(0));
ARGS_6_2.fftlen = coder.typeof(uint32(0));
ARGS_6_2.srate = coder.typeof(0);
ARGS{6}{2} = coder.typeof(ARGS_4_2);
ARGS{6}{2} = coder.cstructname(ARGS{5}{2},'signal_dimensions_t');
ARGS_6_3 = struct;
ARGS_6_3.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_6_3.value = coder.typeof(0,[Inf  Inf],[1 0]);
ARGS{6}{3} = coder.typeof(ARGS_6_3,[Inf  1],[1 0]);
ARGS{6}{3} = coder.cstructname(ARGS{6}{3},'user_config_t');

%% Example coder invocation. Remove the unneeded callbacks
% codegen -config cfg ...
%     -o example_24 ...
%     init -args ARGS{1} ...
%     prepare -args ARGS{2} ...
%     process_ww -args ARGS{3} ...
%     process_ss -args ARGS{4} ...
%     process_ws -args ARGS{5} ...
%     process_sw -args ARGS{6} ...
%     release

%% Invoke MATLAB Coder. Comment out the unneeded processing callbacks
codegen -config cfg...
    -o example_25...
    init -args ARGS{1}...
    prepare -args ARGS{2}...
    process_ss -args ARGS{4}...
    release

%% Optionally package the code for deployment elsewhere
%load('codegen\dll\example_27\buildinfo.mat')
%packNGo(buildInfo,'fileName','example_27.zip');
