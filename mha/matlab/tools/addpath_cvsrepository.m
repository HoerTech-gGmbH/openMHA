function addpath_cvsrepository
  sThis = which(mfilename);
  sPath = fileparts(sThis);
  addpath([sPath,'/../afc_method/']);
  if isunix
    addpath([sPath,'/../i686-linux-gcc-3.3/']);
  end
  addpath([sPath,'/../gt_basic/']);
  addpath([sPath,'/../gt_basic/src']);
  addpath([sPath,'/../gt_basic/binaries']);
  addpath([sPath,'/../gt_basic/data']);
  