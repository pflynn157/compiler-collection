
#OUTPUT
#100
#0
#100
#0
#100
#0
#100
#0
#100
#0
#END

#RET 0

func main -> i32 is
    var numbers : i32[10];
    var i : i32 := 0;
    
    while i < 10 do
        numbers[i] := 0;
        i := i + 1;
    end
    
    i := 0;
    while i < 10 do
        numbers[i] := 100;
        i := i + 2;
    end
    
    i := 0;
    while i < 10 do
        println("%d", numbers[i]);
        i := i + 1;
    end
    
    return 0;
end
