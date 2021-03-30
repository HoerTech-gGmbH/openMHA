function make(iodomain,varargin)
%MAKE Generate C code for the matlab wrapper plugin
% make(IODOMAIN,[OPTIONS]) generates C code for with input and
% output domains specified by IODOMAIN. Valid IODOMAIN
% are:
% 'ww' wave to wave 
% 'ss' for spectrum to spectrum
% 'sw' spectrum to wave processing
% 'ws' wave to spectrum processing.
% Valid OPTIONs are 
% 'packOutput' (logical): Pack the generated code into a zip file
% 'outputName' (string): Name of the generated library
%
% EXAMPLES:
%    make('ww','outputName','example_25','packOutput',true);
packOutput=false;
outputName='';
if mod(nargin-1,2)~=0
    error('Malformed argument list');
end
for i=1:2:nargin-1
    switch lower(varargin{i})
        case 'packoutput'
            packOutput=logical(varargin{i+1});
        case 'outputname'
            outputName=varargin{i+1};
        otherwise
            error('Unknown argument: %s.',varargin{i});
    end
end
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
ARGS{1} = cell(2,1);
ARGS_1_1 = struct;
ARGS_1_1.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_1_1.value = coder.typeof(0,[Inf Inf],[1 1]);
ARGS{1}{1} = coder.typeof(ARGS_1_1,[Inf  1],[1 0]);
ARGS{1}{1} = coder.cstructname(ARGS{1}{1},'user_config_t');
ARGS{1}{2} = ARGS{1}{1};

%% Define argument types for entry-point 'prepare'.
ARGS{2} = cell(3,1);
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
ARGS{2}{3} = ARGS{1}{1};

%% Define argument types for entry-point 'process_ww'.
ARGS{3} = cell(4,1);
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
ARGS{3}{4} = ARGS{3}{3};

%% Define argument types for entry-point 'process_ss'.
ARGS{4} = cell(4,1);
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
ARGS{4}{4} = ARGS{4}{3};

%% Define argument types for entry-point 'process_ws'.
ARGS{5} = cell(4,1);
ARGS{5}{1} = coder.typeof(0,[Inf Inf],[1 1]);
ARGS_5_2 = struct;
ARGS_5_2.channels = coder.typeof(uint32(0));
ARGS_5_2.domain = coder.typeof('X');
ARGS_5_2.fragsize = coder.typeof(uint32(0));
ARGS_5_2.wndlen = coder.typeof(uint32(0));
ARGS_5_2.fftlen = coder.typeof(uint32(0));
ARGS_5_2.srate = coder.typeof(0);
ARGS{5}{2} = coder.typeof(ARGS_5_2);
ARGS{5}{2} = coder.cstructname(ARGS{5}{2},'signal_dimensions_t');
ARGS_5_3 = struct;
ARGS_5_3.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_5_3.value = coder.typeof(0,[Inf  Inf],[1 0]);
ARGS{5}{3} = coder.typeof(ARGS_5_3,[Inf  1],[1 0]);
ARGS{5}{3} = coder.cstructname(ARGS{5}{3},'user_config_t');
ARGS{5}{4} = ARGS{5}{3};

%% Define argument types for entry-point 'process_sw'.
ARGS{6} = cell(4,1);
ARGS{6}{1} = coder.typeof(complex(0),[Inf Inf],[1 1]);
ARGS_6_2 = struct;
ARGS_6_2.channels = coder.typeof(uint32(0));
ARGS_6_2.domain = coder.typeof('X');
ARGS_6_2.fragsize = coder.typeof(uint32(0));
ARGS_6_2.wndlen = coder.typeof(uint32(0));
ARGS_6_2.fftlen = coder.typeof(uint32(0));
ARGS_6_2.srate = coder.typeof(0);
ARGS{6}{2} = coder.typeof(ARGS_6_2);
ARGS{6}{2} = coder.cstructname(ARGS{5}{2},'signal_dimensions_t');
ARGS_6_3 = struct;
ARGS_6_3.name = coder.typeof('X',[1 Inf],[0 1]);
ARGS_6_3.value = coder.typeof(0,[Inf  Inf],[1 0]);
ARGS{6}{3} = coder.typeof(ARGS_6_3,[Inf  1],[1 0]);
ARGS{6}{3} = coder.cstructname(ARGS{6}{3},'user_config_t');
ARGS{6}{4} = ARGS{6}{3};

switch lower(iodomain)
    case 'ww'
        process_name='process_ww';
        args_idx=3;
    case 'ss'
        process_name='process_ss';
        args_idx=4;
    case 'ws'
        process_name='process_ws';
        args_idx=5;
    case 'sw'
        process_name='process_sw';
        args_idx=6;
    otherwise
        error('Unsupported ioDomain: %s. Must be ww, ss, sw, or ws.',ioDomain);
end

%% Invoke MATLAB Coder.
codegen('-config',cfg,...
    '-o',outputName,...
    'init','-args',ARGS{1},...
    'prepare','-args',ARGS{2},...
    process_name,'-args',ARGS{args_idx},...
    'release');

%% Optionally package the code for deployment elsewhere
if(packOutput)
    buildInfo=load(['codegen/dll/' outputName '/buildInfo.mat']);
    packNGo(buildInfo.buildInfo,'fileName',[outputName '.zip']);
end

end
