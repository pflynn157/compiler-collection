
#OUTPUT
#ABCDEFG??
#ABCDEFG??
#END

#RET 0

func printArray(str:char[], size:i32) is
    var i : i32 := 0;
    while i < size do
        print("%c", str[i]);
        i := i + 1;
    end
end

func main -> i32 is
    var c : char := 'A';
    var str : char[10];
    
    var i : i32 := 0;
    while i < 10 do
        if i = 0 then str[0] := 'A';
        elif i = 1 then str[1] := 'B';
        elif i = 2 then str[2] := 'C';
        elif i = 3 then str[3] := 'D';
        elif i = 4 then str[4] := 'E';
        elif i = 5 then str[5] := 'F';
        elif i = 6 then str[6] := 'G';
        elif i = 9 then str[9] := '\n';
        else str[i] := '?';
        end
        
        i := i + 1;
    end
    
    i := 0;
    while i < 10 do
        print("%c", str[i]);
        i := i + 1;
    end
    
    printArray(str, 10);
    
    return 0;
end

