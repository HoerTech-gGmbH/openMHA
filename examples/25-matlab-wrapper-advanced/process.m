function [wave_out,user_config] = process(wave_in,signal_dimensions,user_config)
delay=user_config(1).value;
gain=user_config(2).value;
persistent state;
if(isempty(state))
    state=zeros(signal_dimensions.fragsize+uint32(max(delay(:))),signal_dimensions.channels);
end

persistent read_idx;
if(isempty(read_idx))
    read_idx=uint32(zeros(signal_dimensions.channels));
end

persistent write_idx;
if(isempty(write_idx))
    write_idx=delay;
end

for fr=1:signal_dimensions.fragsize
    for ch=1:signal_dimensions.channels
        write_idx(ch)=mod(write_idx(ch) , (signal_dimensions.fragsize+delay(ch)))+1;
        state(write_idx(ch),ch)=wave_in(fr,ch);
    end
end

wave_out=zeros(signal_dimensions.fragsize,1);
for fr=1:signal_dimensions.fragsize
    for ch=1:signal_dimensions.channels
        read_idx(ch)=mod(read_idx(ch) , (signal_dimensions.fragsize+delay(ch)))+1;
        wave_out(fr)=wave_out(fr)+state(read_idx(ch),ch)*10^(gain(ch)/10);
    end
end
end
