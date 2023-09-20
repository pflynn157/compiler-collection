
#OUTPUT
#*
#**
#***
#****
#*****
#******
#*******
#********
#*********
#**********
#END

#RET 0

func main -> i32 is
    var x : i32 := 0;
    var i : i32 := 0;
    
    while x < 10 do
        i := 1;
        while i <= x + 1 do
            print("*");
            i := i + 1;
        end
        println("");
        
        x := x + 1;
    end
    
    return 0;
end

