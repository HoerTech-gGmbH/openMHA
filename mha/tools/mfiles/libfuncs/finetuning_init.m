function sFT = finetuning_init( f )
  if nargin < 1
    f = 1000*2.^[-2:3];
  end
  sFT = struct;
  sFT.f = f;
  sFT.l = struct('gain',zeros(size(f)),'maxgain',60+zeros(size(f)));
  sFT.r = sFT.l;
