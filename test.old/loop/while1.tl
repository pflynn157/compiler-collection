
#OUTPUT
#X: 0
#X: 1
#X: 2
#X: 3
#X: 4
#X: 5
#^ X: 6
#^ X: 7
#^ X: 8
#^ X: 9
#END

#RET 0

func main -> i32 is
    var x : i32 := 0;
    
    while x < 10 do
        if x > 5 then
            print("^ ");
        end
        
        println("X: %d", x);
        x := x + 1;
    end

    return 0;
end

