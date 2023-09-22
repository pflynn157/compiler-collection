@wordsize 64


func main{args:string[], argc:i32} -> i32 is
    var i : i32 := 0;
    var x : i32 := 0;   

    while i < argc do
        print{args[i] i j x};
        i := i + 1;
    end

    return {x, y, z, 10};
end
