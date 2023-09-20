
#OUTPUT
#0
#1
#2
#3
#4
#7
#8
#9
#END

#RET 0

func main -> i32 is
    var x : i32 := 0;
    
    while x < 10 do
        if x = 5 then
            x := x + 2;
            continue;
        end
        
        println("%d", x);
        x := x + 1;
    end
    
    return 0;
end

