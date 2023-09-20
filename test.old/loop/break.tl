
#OUTPUT
#0
#1
#2
#3
#4
#END

#RET 0

func main -> i32 is
    var x : i32 := 0;
    
    while x < 10 do
        if x = 5 then break; end
        
        println("%d", x);
        x := x + 1;
    end
    
    return 0;
end

