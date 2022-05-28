# Python3 program to illustrate
# Expectimax Algorithm
  
# Structure to declare
# left and right nodes
class Bot:
    def __init__(self, grid):
        self.grid = grid

    def get_move(self):
        node = Node(self.grid)
        up_utility = expectimax(node.options[0], False, 0)
        left_utility = expectimax(node.options[1], False, 0)
        down_utility = expectimax(node.options[2], False, 0)
        right_utility = expectimax(node.options[3], False, 0)
        max_utility = max(up_utility, left_utility, down_utility, right_utility)
        if (max_utility == up_utility):
            return "Up"
        elif (max_utility == left_utility):
            return "Left"
        elif (max_utility == down_utility):
            return "Down"
        else:
            return "Right"
    
class Node:
    def __init__(self, grid):
        self.grid = grid
        self.value = grid.current_score
        self.options = [Node(grid.copy().grid_up()), Node(grid.copy().grid_left()), \
                        Node(grid.copy().grid_down()), Node(grid.copy().grid_right())]
        

# Getting expectimax
def expectimax(node, is_max, depth):
 
    # Condition for Terminal node
    if (depth == 2 or (node.left == None and node.right == None)):
        return node.value
     
    # Maximizer node. Chooses the max from the 4 move subtrees
    if (is_max):
        return max(expectimax(node.options[0], False, depth+1), expectimax(node.options[1], False, depth+1),
                    expectimax(node.options[2], False, depth+1), expectimax(node.options[3], False, depth+1))
  
    # Chance node. Returns the average of possible panel spawns
    else:
        spots = node.grid.retrieve_empty_cells()
        expected_value = 0
        for spot in spots:
            # Create the node representing the possibility a 4 might show up in this empty cell
            four_possibility = Node(node.grid.copy())
            four_possibility.grid.cells[spot[0]][spot[1]] = 4
            # Create the node representing the possibility a 2 might show up in this empty cell
            two_possibility = Node(node.grid.copy())
            two_possibility.grid.cells[spot[0]][spot[1]] = 2
            expectation_sum += (0.9*expectimax(two_possibility, True, depth+1) + 
                                0.1*expectimax(four_possibility, True, depth+1))
        return expectation_sum/len(spots)
     