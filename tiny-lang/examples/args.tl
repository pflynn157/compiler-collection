
func main(args:string[], argc:i32) -> i32 is
    var i : i32 := 0;
    while i < argc do
        println(args[i]);
        i := i + 1;
    end

    return 0;
end
