
#OUTPUT
#0|2|4|6|8|10|12|14|16|18|20|
#Done
#END

#RET 0

func main -> i32 is
    var i : i32 := 0;
    
    while true do
        print("%d|", i);
        i := i + 2;
        
        if i = 22 then break; end
    end
    
    println("\nDone");
    
    return 0;
end

