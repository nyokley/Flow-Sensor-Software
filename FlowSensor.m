sCMD = 0;
CMD = 0;
disp('Command List:')
disp('1. Start Polling')
disp('2. Stop Polling')
disp('3. Read Data')
disp('4. Read Data Status')
disp('5. Read Battery Status')

while CMD < 1 || CMD > 5

CMD = input('Select Command Number: ');

end

sCMD = CMD;

switch CMD
    case 1
        N = input('Enter number of samples to be taken: ');
        sCMD = strcat(sCMD, '_');
        sCMD = strcat(sCMD, num2str(N));
    
end


s = serial('COM10');
fopen(s);
out = '';
fprintf(s,sCMD);
while(1) 
    if(s.BytesAvailable > 0) 
        out = strcat(out, fscanf(s));
        
    elseif (~strcmp(out, ''))
        out
        out = '';
    end
        
end
fclose(s);
delete(s);
clear s;



