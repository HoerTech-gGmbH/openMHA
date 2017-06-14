function [s,vOrder] = sd_reorder_datacol( s, vOrder )
% reorder data fields
% s : input data structure
% vOrder : new order (vector), or new stride (scalar)
  if prod(size(vOrder))==1
    Ndata = numel(s.fields)-numel(s.values);
    Npar = numel(s.values);
    N1 = vOrder;
    N2 = floor(Ndata/N1);
    if N1*N2 ~= Ndata
      error(sprintf('The number of data columns (%d) is not a multiple of vOrder (%d)',...
		    Ndata,N1));
    end
    vOrder = reshape([1:Ndata],N2,N1)';
    vOrder = Npar+vOrder(:)';
  end
  if any(vOrder<=numel(s.values))
    error('vOrder is containing param columns');
  end
  vOrder = [1:numel(s.values),vOrder(:)'];
  s.fields = s.fields(vOrder);
  s.data = s.data(:,vOrder);