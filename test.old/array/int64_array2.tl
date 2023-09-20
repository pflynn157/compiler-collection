
#OUTPUT
#0
#1
#2
#3
#4
#5
#6
#7
#8
#9
#END

#RET 0

func main -> i32 is
    var numbers : i64[10];
    var x : i64 := 0;
    
    var i : i32 := 0;
    while i < 10 do
        numbers[i] := i;
        i := i + 1;
    end
    
    i := 0;
    while i < 10 do
        println("%d", numbers[i]);
        i := i + 1;
    end
    
    return 0;
end

