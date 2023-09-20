extern printf(ln : string, x : i32);

func main -> i32 is
    var c1 : char := 'A';
    var c2 : char := c1;
    var c3 : char := c1 - c2;
    
    return c3;
end

