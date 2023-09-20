
#OUTPUT
#0|1|2|3|4|5|6|7|8|9|
#Done
#END

#RET 0

func main -> i32 is
    var i : i32 := 0;
    while i < 10 do
        print("%d|", i);
        i := i + 1;
    end
    
    println("\nDone");
    
    return 0;
end

