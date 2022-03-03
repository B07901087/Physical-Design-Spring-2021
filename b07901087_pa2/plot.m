%author: Tsai, Pei Huan
filename = 'forplot.txt';
filename2 = 'ami49.block';
contents = textread(filename,'%s','delimiter','\n','whitespace','');
contents2 = textread(filename2, '%s', 'delimiter', '\n', 'whitespace', '');
lines = [];
lines2 = string(contents2(1));
outline = split(lines2(1));
for i = 6:length(contents)
    lines = [lines string(contents(i))];
end

figure(1); 
outline = str2double(outline);
rectangle('Position', [0, 0, outline(2), outline(3)], 'EdgeColor','black','FaceColor','white');
for i = 1:length(lines)
   tmp = split(lines(i));
   name = tmp(1,1);
   x = str2double(tmp(2,1));
   y = str2double(tmp(3,1));
   w = str2double(tmp(4,1))-x;
   h = str2double(tmp(5,1))-y;
   rectangle('Position',[x,y,w,h],'Edgecolor', 'b','FaceColor','c'); 
   text(x+w/3,y+h/2,name,'Color','red','FontSize',7)
end

title(filename);