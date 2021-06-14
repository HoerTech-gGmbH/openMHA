function handle = sidelink(src,event)
handle = guidata(src);
stat = get(handle.link,'tag');
if isequal(stat,'link')
    set(handle.link,'tag','nolink','string','link');
else
    set(handle.link,'tag','link','string','unlink');
end
end
