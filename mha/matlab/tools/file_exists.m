function b = file_exists(filename)

fd = fopen(filename);
if (fd < 0)
    b = false;
else
    fclose(fd);
    b = true;
end
