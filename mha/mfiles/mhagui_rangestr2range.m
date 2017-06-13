function [v_min, v_max] = mhagui_rangestr2range( srange, val )
  [range,c] = sscanf(srange,'%c %f , %f %c');
  if c == 4
    if isempty(find(range(1)=='[]'))
      error('Invalid range delimiter');
    end
    if isempty(find(range(4)=='[]'))
      error('Invalid range delimiter');
    end
    v_min = range(2);
    v_max = range(3);
    if range(1) == ']'
      v_min = v_min + eps;
    end
    if range(4) == '['
      v_max = v_max - eps;
    end
  else
    [range,c] = sscanf(srange,'%c , %f %c');
    if c == 3
      if isempty(find(range(1)=='[]'))
	error('Invalid range delimiter');
      end
      if isempty(find(range(3)=='[]'))
	error('Invalid range delimiter');
      end
      v_max = range(2);
      v_min = val-4*(v_max-val);
      if v_max == v_min 
	v_min = v_max - 7;
      end
    else
      [range,c] = sscanf(srange,'%c %f , %c');
      if c == 3
	if isempty(find(range(1)=='[]'))
	  error('Invalid range delimiter');
	end
	if isempty(find(range(3)=='[]'))
	  error('Invalid range delimiter');
	end
	v_min = range(2);
	v_max = val+4*(val-v_min);
	if v_max == v_min 
	  v_max = v_min + 7;
	end
      else
	v_min = val-1;
	v_max = val+1;
      end
    end
  end

