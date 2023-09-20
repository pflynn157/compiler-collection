
#OUTPUT
#0
#2
#4
#6
#8
#10
#12
#14
#16
#18
#END

#RET 0

func main -> i32 is
    var numbers : i32[10];
    var x : i32 := 0;
    
    var i : i32 := 0;
    while i < 10 do
        numbers[i] := i * 2;
        i := i + 1;
    end
    
    i := 0;
    while i < 10 do
        println("%d", numbers[i]);
        i := i + 1;
    end
    
    return 0;
end

