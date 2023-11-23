
#
# A simple tic-tac-toe game written in Tiny
#
import std.io;

func print_board(positions:char[]) is
    # 0 | 1 | 2
    #-----------
    # 3 | 4 | 5
    #-----------
    # 6 | 7 | 8
    var sp : char := ' ';
    println(" %c | %c | %c ", positions[0], positions[1], positions[2]);
    println("------------");
    println(" %c | %c | %c ", positions[3], positions[4], positions[5]);
    println("------------");
    println(" %c | %c | %c ", positions[6], positions[7], positions[8]);
end

func check_combo(positions:char[], player:char, p1:i32, p2:i32, p3:i32) -> bool is
    if positions[p1] = player and positions[p2] = player and positions[p3] = player then
        return true;
    end
    
    return false;
end

# Winning combos
# 0, 1, 2
# 3, 4, 5
# 6, 7, 8
# 0, 3, 6
# 1, 4, 7
# 2, 5, 8
# 0, 4, 8
# 2, 4, 6
func check_winner(positions:char[], player:char) -> bool is
    # Combo 1 -> 0, 1, 2
    if check_combo(positions, player, 0, 1, 2) then
        return true;
    end
    
    # 3, 4, 5
    if check_combo(positions, player, 3, 4, 5) then
        return true;
    end
    
    # 6, 7, 8
    if check_combo(positions, player, 6, 7, 8) then
        return true;
    end
    
    # 0, 3, 6
    if check_combo(positions, player, 0, 3, 6) then
        return true;
    end
    
    # 1, 4, 7
    if check_combo(positions, player, 1, 4, 7) then
        return true;
    end
    
    # 2, 5, 8
    if check_combo(positions, player, 2, 5, 8) then
        return true;
    end
    
    # 0, 4, 8
    if check_combo(positions, player, 0, 4, 8) then
        return true;
    end
    
    # 2, 4, 6
    if check_combo(positions, player, 2, 4, 6) then
        return true;
    end
    
    return false;
end

# Checks for a draw
func check_draw(positions:char[]) -> bool then
    var i : i32 := 0;
    while i < 9 do
        if positions[i] != 'X' and positions[i] != 'O' then
            return false;
        end
        
        i := i + 1;
    end
    
    return true;
end

func main(args:string[], argc:i32) -> i32 is
    var positions : char[9];
    var i : i32 := 0;
    while i < 9 do
        positions[i] := i + '0';
        i := i + 1;
    end
    
    print_board(positions);
    
    var winner, draw : bool := false;
    var user_num : i32 := 1;
    while true do
        # Print the current player
        var player : char := 0;
        if user_num = 1 then player := 'X';
        elif user_num = 2 then player := 'O';
        end
        println("\nCurrent player: %c", player);
        
        # Position on the board
        var pos : i32 := 0;
        print("Enter board position: ");
        pos := readint();
        
        var should_switch : bool := true;
        
        if pos >= 0 and pos < 9 then
            if positions[pos] = 'X' or positions[pos] = 'O' then
                println("Position already taken!");
                should_switch := false;
            else
                positions[pos] := player;
                print_board(positions);
            end
        else
            should_switch := false;
        end
        
        # Check the winner
        winner := check_winner(positions, player);
        if winner = true then
            break;
        end
        
        # Check for a draw
        draw := check_draw(positions);
        if draw = true then
            break;
        end
        
        # Switch user
        if should_switch then
            if user_num = 2 then user_num := 1;
            else user_num := 2;
            end
        end
    end
    
    if draw then
        println("Draw");
    else
        println("\nWinner: %c", player);
    end

    return 0;
end
